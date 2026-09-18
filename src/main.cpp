#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BluetoothManager.hpp>
#include <Preferences.h>
#include <PrefKeys.hpp>
#include <LokitAPI.hpp>
#include <LedManager.hpp>
#include <MqttManager.hpp>
#include <DeviceStatus.hpp>

#define SS_PIN 5
#define RST_PIN 21

#define BTN_PIN 22

Preferences preferences;
MFRC522 rfid(SS_PIN, RST_PIN);
BluetoothManager* bluetooth = nullptr;
LedManager* led = nullptr;
LokitAPI* api = nullptr;
MqttManager* mqtt = nullptr;

DeviceStatus devStatus = DeviceStatus::IDLE;
int lastProvBtnState = HIGH;

String readCardUid(){
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uidString += "0"; 
    }
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();

  return uidString;
}

void refreshConfig(){
  String ssid = preferences.getString(WIFI_SSID_KEY, "");
  String pass = preferences.getString(WIFI_PASS_KEY, "");
  String serverName = preferences.getString(LOKIT_SERVER_KEY, "");
  int serverHttpPort = preferences.getInt(LOKIT_SERVER_HTTP_PORT_KEY, 80);
  int serverMqttPort = preferences.getInt(LOKIT_SERVER_MQTT_PORT_KEY, 1883);
  String deviceToken = preferences.getString(LOKIT_TOKEN_KEY, "");

  Serial.printf("Lokit server: %s (HTTP %d, MQTT %d)\n", serverName.c_str(), serverHttpPort, serverMqttPort);

  const bool wifiReady = !ssid.isEmpty() && !pass.isEmpty();
  const bool lokitReady = !serverName.isEmpty() && !deviceToken.isEmpty();

  if (wifiReady) {
    // WiFi init
    WiFi.begin(ssid, pass);
    Serial.println("Connecting to WiFi");

    int waits = 0;
    while(!WiFi.isConnected()){
      if(waits == 10)
        break;

      delay(1000);
      Serial.print('.');
      waits++;
    }
  }

  const bool wifiConnected = WiFi.isConnected();

  if(wifiReady && !wifiConnected) Serial.println("Couldn't connect to WiFi!");
  else if (wifiReady) {
    Serial.println("WiFi connected!");
    Serial.printf("IPV4: ");
    Serial.println(WiFi.localIP());
    Serial.printf("MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.println();
  }

  if (lokitReady) api->init(serverName, serverHttpPort, deviceToken);
  if (lokitReady) mqtt->init(serverName, serverMqttPort, deviceToken);

  devStatus = wifiReady && lokitReady && wifiConnected ? DeviceStatus::IDLE : DeviceStatus::NOT_CONF;
  if(devStatus == DeviceStatus::NOT_CONF) Serial.println("Device not fully configured! Start BLE provisioning and upload the configuration!");
}



void setup() {
  Serial.begin(115200);
  Serial.println("=== LOKIT READER INIT ===");

  //hardware init
  pinMode(BTN_PIN, INPUT_PULLUP);

  SPI.begin();
  rfid.PCD_Init();

  // config init
  preferences.begin("lokit-reader", false);
  bluetooth = new BluetoothManager(&preferences);
  led = new LedManager(devStatus);
  api = new LokitAPI();
  mqtt = new MqttManager(&preferences);
  bluetooth->initBLE();
  led->init();

  refreshConfig();
  Serial.println("LOKIT READER INIT COMPLETE");
}

void loop() {
  int currentProvBtnState = digitalRead(BTN_PIN);

  if(lastProvBtnState == LOW && currentProvBtnState == HIGH){
    if(bluetooth->isProvInProgress()){
      Serial.println("Stopping BLE provisioning...");
      bluetooth->stopProv();
      refreshConfig();
    }
    else {
      Serial.println("Starting BLE provisioning...");
      bluetooth->startProv();
      devStatus = DeviceStatus::IN_PROV;
    }
  }

  lastProvBtnState = currentProvBtnState;

  if (devStatus == DeviceStatus::IDLE && !mqtt->loop()) {
    Serial.println("MQTT token is invalid. Device not fully configured!");
    devStatus = DeviceStatus::NOT_CONF;
  }

  switch(devStatus){
    case DeviceStatus::IN_PROV:
    case DeviceStatus::NOT_CONF:
      return;
    default:
      break;
  }

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uidString = readCardUid();
  Serial.printf("Card detected: %s\n", uidString);

  Serial.println("Trying to get decision from server...");
  DecisionOutcome out = api->requestDecision(uidString);

  switch (out){
    case DecisionOutcome::ACCESS_OK: {
      Serial.println("ACCESS GRANTED");
      devStatus = DeviceStatus::OPEN;
      delay(1000);
      break;
    }
    case DecisionOutcome::ACCESS_DENIED: {
      Serial.println("ACCESS DENIED");
      devStatus = DeviceStatus::ENTRY_DENIED;
      delay(1000);
      break;
    }
    case DecisionOutcome::TOKEN_REVOKED: {
      Serial.println("Device token has been revoked. Please start PROV and upload a new one!");
      devStatus = DeviceStatus::NOT_CONF;
      return;
    }
    case DecisionOutcome::CONN_ERR: {
      Serial.println("Network error while contacting the API. Check WiFi or server availability.");
      devStatus = DeviceStatus::NETWORK_ERR;
      delay(2000);
      break;
    }
    case DecisionOutcome::UNKNOWN_CODE: {
      Serial.println("API returned an unknown response code. Potential server issue, no need to PROV yet");
      break;
    }
    case DecisionOutcome::MALFORMED_BODY: {
      Serial.println("API returned a malformed body with status 200. Potential server issue, no need to PROV yet");
      break;
    }
  }

  devStatus = DeviceStatus::IDLE;
  rfid.PICC_HaltA();
}
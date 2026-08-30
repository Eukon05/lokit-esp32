#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BluetoothManager.hpp>
#include <Preferences.h>
#include <PrefKeys.hpp>
#include <LokitAPI.hpp>
#include <DeviceStatus.hpp>

#define SS_PIN 5
#define RST_PIN 21

#define BTN_PIN 22

#define LED_R 12
#define LED_G 14
#define LED_B 27

Preferences preferences;
MFRC522 rfid(SS_PIN, RST_PIN);
BluetoothManager* bluetooth = nullptr;
LokitAPI* api = nullptr;

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
  String deviceToken = preferences.getString(LOKIT_TOKEN_KEY, "");

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

  if (lokitReady) api->init(serverName, deviceToken);

  devStatus = wifiReady && lokitReady && wifiConnected ? DeviceStatus::IDLE : DeviceStatus::NOT_CONF;
  if(devStatus == DeviceStatus::NOT_CONF) Serial.println("Device not fully configured! Start BLE provisioning and upload the configuration!");
}

void setLedColor(int r, int g, int b){
  digitalWrite(LED_R, r);
  digitalWrite(LED_G, g);
  digitalWrite(LED_B, b);
  delay(100);
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== LOKIT READER INIT ===");

  //hardware init
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  SPI.begin();
  rfid.PCD_Init();

  // config init
  preferences.begin("lokit-reader", false);
  bluetooth = new BluetoothManager(&preferences);
  api = new LokitAPI();
  bluetooth->initBLE();

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

  switch(devStatus){
    case DeviceStatus::IDLE: {
      setLedColor(0, 0, 0);
      break;
    }
    case DeviceStatus::IN_PROV: {
      setLedColor(0, 0, 50);
      return;
    }
    case DeviceStatus::NOT_CONF: {
      setLedColor(50, 50, 0);
      return;
    }
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
      setLedColor(0, 50, 0);
      delay(1000);
      break;
    }
    case DecisionOutcome::ACCESS_DENIED: {
      Serial.println("ACCESS DENIED");
      setLedColor(50, 0,0);
      delay(1000);
      break;
    }
    case DecisionOutcome::TOKEN_REVOKED: {
      Serial.println("Device token has been revoked. Please start PROV and upload a new one!");
      devStatus = DeviceStatus::NOT_CONF;
      break;
    }
    case DecisionOutcome::CONN_ERR: {
      Serial.println("Network error while contacting the API. Check WiFi or server availability.");
      setLedColor(50, 0, 0);
      delay(500);
      setLedColor(50, 50, 0);
      delay(500);
      setLedColor(50, 0, 0);
      delay(500);
      setLedColor(50, 50, 0);
      delay(500);
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
  
  rfid.PICC_HaltA();
}
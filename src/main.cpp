#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BluetoothManager.hpp>
#include <Preferences.h>
#include <PrefKeys.hpp>
#include <LokitAPI.hpp>

#define SS_PIN 5
#define RST_PIN 21

#define BTN_PIN 22

Preferences preferences;
MFRC522 rfid(SS_PIN, RST_PIN);
BluetoothManager* bluetooth = nullptr;
LokitAPI* api = nullptr;

bool ready = true;
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

  if(wifiReady && !wifiConnected){
    Serial.println("Couldn't connect to WiFi!");
    preferences.remove(WIFI_PASS_KEY);
    ESP.restart();
  }
  else if (wifiReady) {
    Serial.println("WiFi connected!");
    Serial.printf("IPV4: ");
    Serial.println(WiFi.localIP());
    Serial.printf("MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.println();
  }

  if (lokitReady) {
    api = new LokitAPI(serverName, deviceToken);
  }

  ready = wifiReady && lokitReady && wifiConnected;

  if(!ready){
    Serial.println("Device not fully configured! Starting BLE provisioning...");
    if (!wifiConnected || !lokitReady) bluetooth->initBLE(!wifiConnected, !lokitReady);
  }

  Serial.println("LOKIT READER INIT COMPLETE");
}

void loop() {
  int currentProvBtnState = digitalRead(BTN_PIN);

  if(lastProvBtnState == LOW && currentProvBtnState == HIGH)
    Serial.println("ENABLING PROV"); // placeholder

  lastProvBtnState = currentProvBtnState;

  if(!ready) return;
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uidString = readCardUid();
  Serial.printf("Card detected: %s\n", uidString);

  Serial.println("Trying to get decision from server...");
  DecisionOutcome out = api->requestDecision(uidString);

  switch (out){
    case DecisionOutcome::OK: {
      Serial.println("ACCESS GRANTED");
      break;
    }
    case DecisionOutcome::DENIED: {
      Serial.println("ACCESS DENIED");
      break;
    }
    case DecisionOutcome::TOKEN_REVOKED: {
      Serial.println("Device token has been revoked. Please start PROV and upload a new one!");
      ready = false;
      break;
    }
    case DecisionOutcome::CONN_ERR: {
      Serial.println("Network error while contacting the API. Check WiFi or server availability.");
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
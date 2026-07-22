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
 
Preferences preferences;
MFRC522 rfid(SS_PIN, RST_PIN);
BluetoothManager* bluetooth = nullptr;
LokitAPI* api = nullptr;

bool ready = true;

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

  bluetooth = new BluetoothManager(&preferences);

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
  if(!ready) return;
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uidString = readCardUid();
  Serial.printf("Card detected: %s\n", uidString);

  Serial.println("Trying to get decision from server...");
  api->requestDecision(uidString);
  
  rfid.PICC_HaltA();
}
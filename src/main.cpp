#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <LedManager.hpp>
#include <ProvButtonManager.hpp>
#include <DeviceManager.hpp>

#define SS_PIN 5
#define RST_PIN 21

Preferences preferences;
MFRC522 rfid(SS_PIN, RST_PIN);

LedManager* led = nullptr;
ProvButtonManager* provBtnMgr = nullptr;
DeviceManager* device = nullptr;

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

  SPI.begin();
  rfid.PCD_Init();

  // config init
  preferences.begin("lokit-reader", false);
  device = new DeviceManager(&preferences);

  led = new LedManager(device->getDeviceStatus());
  provBtnMgr = new ProvButtonManager(device);

  led->startLoop();

  device->init();
  Serial.println("LOKIT READER INIT COMPLETE");
}

void loop() {
  device->processRequests();

  switch(device->getDeviceStatus()){
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
  DecisionOutcome out = device->getLokitAPI()->requestDecision(uidString);

  switch (out){
    case DecisionOutcome::ACCESS_OK: {
      Serial.println("ACCESS GRANTED");
      device->setDeviceStatus(DeviceStatus::OPEN);
      delay(1000);
      break;
    }
    case DecisionOutcome::ACCESS_DENIED: {
      Serial.println("ACCESS DENIED");
      device->setDeviceStatus(DeviceStatus::ENTRY_DENIED);
      delay(1000);
      break;
    }
    case DecisionOutcome::TOKEN_REVOKED: {
      Serial.println("Device token has been revoked. Please start PROV and upload a new one!");
      device->setDeviceStatus(DeviceStatus::NOT_CONF);
      return;
    }
    case DecisionOutcome::CONN_ERR: {
      Serial.println("Network error while contacting the API. Check WiFi or server availability.");
      device->setDeviceStatus(DeviceStatus::NETWORK_ERR);
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

  device->setDeviceStatus(DeviceStatus::IDLE);
  rfid.PICC_HaltA();
}
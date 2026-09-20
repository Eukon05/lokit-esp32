#include <LedManager.hpp>

LedManager::LedManager(DeviceStatus &devStat) : deviceStatus(devStat) {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
}

void LedManager::setLedColor(int r, int g, int b) {
    digitalWrite(LED_R, r);
    digitalWrite(LED_G, g);
    digitalWrite(LED_B, b);
}

void LedManager::runLoop(void *parameter) {
    LedManager* instance = static_cast<LedManager*>(parameter);

    while (true) {
        switch (instance->deviceStatus) {
            case DeviceStatus::IDLE: {
                instance->setLedColor(0, 0, 0);
                break;
            }
            case DeviceStatus::IN_PROV: {
                instance->setLedColor(0, 0, 50);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                instance->setLedColor(0, 0, 0);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;
            }
            case DeviceStatus::NOT_CONF: {
                instance->setLedColor(50, 50, 0);
                break;
            }
            case DeviceStatus::OPEN: {
                instance->setLedColor(0, 50, 0);
                break;
            }
            case DeviceStatus::ENTRY_DENIED: {
                instance->setLedColor(50, 0, 0);
                break;
            }
            case DeviceStatus::NETWORK_ERR: {
                instance->setLedColor(50, 0, 0);
                vTaskDelay(500 / portTICK_PERIOD_MS);
                instance->setLedColor(50, 50, 0);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }
    }
}

void LedManager::startLoop(){
    xTaskCreate(
    runLoop,         // Task function
    "LedTask",       // Task name
    10000,             // Stack size (bytes)
    this,              // Parameters
    1,                 // Priority
    &ledTaskHandle  // Task handle
  );
}
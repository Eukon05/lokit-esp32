#pragma once

#include <Arduino.h>

class DeviceManager;

#define BTN_PIN 22

class ProvButtonManager {
    private:
    TaskHandle_t provBtnTaskHandle = nullptr;
    DeviceManager* deviceManager;
    static void runLoop(void *parameter);

    public:
    ProvButtonManager(DeviceManager* deviceManager);
};
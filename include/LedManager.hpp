#include <Arduino.h>
#include <DeviceStatus.hpp>

#define LED_R 12
#define LED_G 14
#define LED_B 27

class LedManager {
    private:
        TaskHandle_t ledTaskHandle = nullptr;
        DeviceStatus& deviceStatus;
        static void runLed(void *parameter);
    public:
        LedManager(DeviceStatus& devStat);
        void init();
        void setLedColor(int r, int g, int b);
};
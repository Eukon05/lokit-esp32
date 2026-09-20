#include <ProvButtonManager.hpp>
#include <DeviceManager.hpp>
#include <DeviceStatus.hpp>

void ProvButtonManager::runLoop(void *parameter)
{
    ProvButtonManager *instance = static_cast<ProvButtonManager *>(parameter);
    int lastProvBtnState = HIGH;

    while (true)
    {
        int currentProvBtnState = digitalRead(BTN_PIN);

        if (lastProvBtnState == HIGH && currentProvBtnState == LOW)
        {
            instance->deviceManager->requestProvToggle();
        }

        lastProvBtnState = currentProvBtnState;
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

ProvButtonManager::ProvButtonManager(DeviceManager *device) : deviceManager(device)
{
    pinMode(BTN_PIN, INPUT_PULLUP);
    xTaskCreate(
        runLoop,
        "ProvBtnTask",
        1000,
        this,
        1,
        &provBtnTaskHandle);
}
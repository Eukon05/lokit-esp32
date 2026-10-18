#include <ServoManager.hpp>

ServoManager::ServoManager(DeviceStatus& devStat) : deviceStatus(devStat) {
    pinMode(SERVO_PIN, OUTPUT);
    servoMotor.attach(SERVO_PIN);
}

void ServoManager::runLoop(void *parameter){
    ServoManager* instance = static_cast<ServoManager*>(parameter);
    
    while (true){
        switch(instance->deviceStatus){
            case DeviceStatus::OPEN:
            case DeviceStatus::KEEP_OPEN: {
                instance->servoMotor.write(OPEN_ANGLE);
                break;
            }
            default: {
                instance->servoMotor.write(CLOSED_ANGLE);
                break;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void ServoManager::startLoop(){
    xTaskCreate(
    runLoop,         // Task function
    "ServoTask",       // Task name
    10000,             // Stack size (bytes)
    this,              // Parameters
    1,                 // Priority
    &servoTaskHandle  // Task handle
  );
}
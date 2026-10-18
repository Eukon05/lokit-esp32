#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include <DeviceStatus.hpp>

#define SERVO_PIN 17
#define OPEN_ANGLE 180
#define CLOSED_ANGLE 0

class ServoManager {
    private:
        Servo servoMotor;
        TaskHandle_t servoTaskHandle = nullptr;
        DeviceStatus& deviceStatus;
        static void runLoop(void *parameter);
    public:
        ServoManager(DeviceStatus& devStat);
        TaskHandle_t mqttTaskHandle = nullptr;
        void startLoop();
};
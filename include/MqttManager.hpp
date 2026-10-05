#pragma once

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <PrefKeys.hpp>
#include <DeviceConfig.hpp>
#include <DeviceStatus.hpp>

class MqttManager
{
private:
    String clientId;
    String mqttHost;
    String clientPass;
    String heartbeatTopic;
    String commandTopic;
    String responseTopic;
    WiFiClient mqttWifi;
    PubSubClient mqttClient;
    DeviceStatus& deviceStatus;
    bool configured = false;
    TaskHandle_t mqttTaskHandle = nullptr;

    void reconnect();
    void callback(char* topic, byte* payload, unsigned int length);
    void publishHeartbeat();
    static void runLoop(void *parameter);

public:
    MqttManager(DeviceStatus& deviceStatus);
    void init(String serverName, int serverPort, String clientPass);
    void startLoop();
};
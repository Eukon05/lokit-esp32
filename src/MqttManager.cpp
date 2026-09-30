#include <MqttManager.hpp>

MqttManager::MqttManager(DeviceStatus& deviceStatus): mqttClient(mqttWifi), deviceStatus(deviceStatus) {
    clientId = WiFi.macAddress();
    clientId.toUpperCase();
    heartbeatTopic = "lokit/devices/" + clientId + "/heartbeat";
}

void MqttManager::init(String serverName, int serverPort, String clientPass)
{
    mqttHost = serverName;
    this->clientPass = clientPass;
    configured = !mqttHost.isEmpty() && !this->clientPass.isEmpty();
    mqttClient.setServer(mqttHost.c_str(), serverPort);
}

void MqttManager::reconnect()
{
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        clientId.toUpperCase();

        if (mqttClient.connect(clientId.c_str(), clientId.c_str(), clientPass.c_str()))
        {
            Serial.println("MQTT Connected");
            deviceStatus = DeviceStatus::IDLE;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());

            switch (mqttClient.state()){
                case MQTT_CONNECT_BAD_CREDENTIALS: 
                case MQTT_CONNECT_UNAUTHORIZED: {
                    Serial.println(" invalid MQTT credentials");
                    configured = false;
                    deviceStatus = DeviceStatus::NOT_CONF;
                    return;
                }
                case MQTT_CONNECTION_TIMEOUT:
                case MQTT_CONNECT_FAILED:
                case MQTT_CONNECT_UNAVAILABLE: {
                    Serial.println(" can't connect to MQTT server");
                    deviceStatus = DeviceStatus::NETWORK_ERR;
                    break;
                }
            }

            Serial.println(" try again in 5 seconds");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

void MqttManager::runLoop(void *parameter)
{
    MqttManager* instance = static_cast<MqttManager*>(parameter);
    unsigned long lastHeartbeatAt = 0;

    while (true) {
        if (!instance->configured || !WiFi.isConnected()){
            if (instance->configured && !WiFi.isConnected()) {
                instance->deviceStatus = DeviceStatus::NETWORK_ERR;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        if (!instance->mqttClient.connected())
        {
            instance->reconnect();
        }

        instance->mqttClient.loop();

        if (millis() - lastHeartbeatAt >= 300000UL)
        {
            instance->publishHeartbeat();
            lastHeartbeatAt = millis();
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void MqttManager::publishHeartbeat(){
    mqttClient.publish(heartbeatTopic.c_str(), "");
    Serial.println("Published heartbeat signal to MQTT");
}

void MqttManager::startLoop() {
    xTaskCreate(
    runLoop,         // Task function
    "MqttTask",       // Task name
    10000,             // Stack size (bytes)
    this,              // Parameters
    1,                 // Priority
    &mqttTaskHandle  // Task handle
  );
}
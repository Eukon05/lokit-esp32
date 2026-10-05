#include <MqttManager.hpp>
#include <ArduinoJson.h>
#include <time.h>

MqttManager::MqttManager(DeviceStatus& deviceStatus): mqttClient(mqttWifi), deviceStatus(deviceStatus) {
    clientId = WiFi.macAddress();
    clientId.toUpperCase();
    heartbeatTopic = "lokit/devices/" + clientId + "/heartbeat";
    commandTopic = "lokit/devices/" + clientId + "/command";
    responseTopic = "lokit/devices/" + clientId + "/response";
}

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void MqttManager::init(String serverName, int serverPort, String clientPass)
{
    mqttHost = serverName;
    this->clientPass = clientPass;
    configured = !mqttHost.isEmpty() && !this->clientPass.isEmpty();
    mqttClient.setServer(mqttHost.c_str(), serverPort);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        callback(topic, payload, length);
    });
}

void MqttManager::callback(char* topic, byte* payload, unsigned int length) {
    // handle incoming command
    if(strcmp(topic, commandTopic.c_str()) == 0){
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error) {
            Serial.print("Invalid MQTT command JSON: ");
            Serial.println(error.c_str());
            return;
        }

        JsonDocument response;
        String responseString;
        response["commandId"] = doc["id"];
        const unsigned long expiresAt = doc["expiresAt"].as<unsigned long>();

        if(expiresAt < getTime()){
            Serial.println("Received an EXPIRED MQTT command, skipping...");
            response["response"] = "EXPIRED";
        }
        else {
            const char* command = doc["command"];
            Serial.printf("Received an MQTT command: %s\n", command);
            response["response"] = "ACK";

            if(strcmp(command, "KEEP_OPEN") == 0){
                deviceStatus = DeviceStatus::KEEP_OPEN;
            }
            else if (strcmp(command, "KEEP_CLOSED") == 0){
                deviceStatus = DeviceStatus::KEEP_CLOSED;
            }
            else if (strcmp(command, "IDLE") == 0){
                deviceStatus = DeviceStatus::IDLE;
            }

            delete command;
        }

        char buffer[256];
        size_t n = serializeJson(response, buffer);
        mqttClient.publish(responseTopic.c_str(), buffer, n);
        Serial.println("Published MQTT command response");

        delete buffer;
    }
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
            mqttClient.subscribe(commandTopic.c_str());
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
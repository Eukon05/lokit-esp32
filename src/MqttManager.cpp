#include <MqttManager.hpp>

MqttManager::MqttManager(Preferences* preferences): prefs(preferences), mqttClient(mqttWifi) {
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

bool MqttManager::reconnect()
{
    int retries = 0;
    while (!mqttClient.connected())
    {
        if(retries > 5)
            return false;

        retries++;
        Serial.print("Attempting MQTT connection...");
        clientId.toUpperCase();

        String pass = prefs->getString(LOKIT_TOKEN_KEY);

        if (mqttClient.connect(clientId.c_str(), clientId.c_str(), pass.c_str()))
        {
            Serial.println("MQTT Connected");
            publishHeartbeat();
            lastHeartbeatAt = millis();
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());

            if (mqttClient.state() == MQTT_CONNECT_BAD_CREDENTIALS ||
                mqttClient.state() == MQTT_CONNECT_UNAUTHORIZED)
            {
                Serial.println(" invalid MQTT credentials");
                configured = false;
                return false;
            }

            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }

    return true;
}

bool MqttManager::loop()
{
    if (!configured || !WiFi.isConnected())
        return configured;

    if (!mqttClient.connected())
    {
        if (!reconnect())
            return false;
    }

    mqttClient.loop();

    if (millis() - lastHeartbeatAt >= 300000UL)
    {
        publishHeartbeat();
        lastHeartbeatAt = millis();
    }

    return true;
}

void MqttManager::publishHeartbeat(){
    mqttClient.publish(heartbeatTopic.c_str(), "");
    Serial.println("Published heartbeat signal to MQTT");
}
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <PrefKeys.hpp>

class MqttManager
{
private:
    String clientId;
    String clientPass;
    String heartbeatTopic;
    String mqttHost;
    WiFiClient mqttWifi;
    PubSubClient mqttClient;
    Preferences *prefs;
    bool configured = false;
    unsigned long lastHeartbeatAt = 0;

    bool reconnect();
    void publishHeartbeat();

public:
    MqttManager(Preferences *preferences);
    void init(String serverName, int serverPort, String clientPass);
    bool loop();
};
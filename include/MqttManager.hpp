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
    TaskHandle_t mqttTaskHandle = nullptr;

    void reconnect();
    void publishHeartbeat();
    static void runLoop(void *parameter);

public:
    MqttManager(Preferences *preferences);
    void init(String serverName, int serverPort, String clientPass);
    void startLoop();
};
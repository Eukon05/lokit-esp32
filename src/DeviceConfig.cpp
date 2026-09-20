#include <DeviceConfig.hpp>

String DeviceConfig::getWifiSsid() {
    return wifiSsid;
}

String DeviceConfig::getWifiPass() {
    return wifiPass;
}

String DeviceConfig::getServerAddress() {
    return serverAddress;
}

int DeviceConfig::getHttpPort() {
    return httpPort;
}

int DeviceConfig::getMqttPort() {
    return mqttPort;
}

String DeviceConfig::getDeviceToken() {
    return deviceToken;
}

DeviceStatus& DeviceConfig::getDeviceStatus() {
    return status;
}

DeviceConfig::DeviceConfig(Preferences* preferences) : preferences(preferences) {};

void DeviceConfig::loadConfig() {
    wifiSsid = preferences->getString(WIFI_SSID_KEY, "");
    wifiPass = preferences->getString(WIFI_PASS_KEY, "");
    serverAddress = preferences->getString(LOKIT_SERVER_KEY, "");
    httpPort = preferences->getInt(LOKIT_SERVER_HTTP_PORT_KEY, 80);
    mqttPort = preferences->getInt(LOKIT_SERVER_MQTT_PORT_KEY, 1883);
    deviceToken = preferences->getString(LOKIT_TOKEN_KEY, "");
}

bool DeviceConfig::isWifiReady(){
    return !wifiSsid.isEmpty() && !wifiPass.isEmpty();
}

bool DeviceConfig::isLokitReady(){
    return !serverAddress.isEmpty() && !deviceToken.isEmpty();
}

void DeviceConfig::setDeviceStatus(DeviceStatus status){
    this->status = status;
}
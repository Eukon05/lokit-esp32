#pragma once

#include <Preferences.h>
#include <PrefKeys.hpp>
#include <DeviceStatus.hpp>

class DeviceConfig {
    private:
        DeviceStatus status = DeviceStatus::IDLE;
        Preferences* preferences;
        String wifiSsid;
        String wifiPass;
        String serverAddress;
        int httpPort;
        int mqttPort;
        String deviceToken;
    public:
        DeviceConfig(Preferences* preferences);
        String getWifiSsid();
        String getWifiPass();
        String getServerAddress();
        int getHttpPort();
        int getMqttPort();
        String getDeviceToken();
        DeviceStatus& getDeviceStatus();
        void setDeviceStatus(DeviceStatus status);
        void loadConfig();
        bool isWifiReady();
        bool isLokitReady();
};
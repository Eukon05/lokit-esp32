#pragma once

#include <DeviceConfig.hpp>
#include <BluetoothManager.hpp>
#include <LokitAPI.hpp>
#include <MqttManager.hpp>
#include <LedManager.hpp>

#define NTP_SERVER "pool.ntp.org"

class DeviceManager {
    private:
    DeviceConfig* config;
    BluetoothManager* bluetoothManager;
    LokitAPI* lokitAPI;
    MqttManager* mqttManager;
    LedManager* ledManager;
    volatile bool provToggleRequested = false;

    public:
    DeviceManager(Preferences* preferences);
    void init();
    void setDeviceStatus(DeviceStatus status);
    DeviceStatus& getDeviceStatus();
    LokitAPI* getLokitAPI();
    void startProv();
    void stopProv();
    void toggleProv();
    void requestProvToggle();
    void processRequests();
};
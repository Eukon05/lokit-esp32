#include <DeviceManager.hpp>

DeviceManager::DeviceManager(Preferences* preferences){
    this->config = new DeviceConfig(preferences);
    this->bluetoothManager = new BluetoothManager(preferences);
    this->bluetoothManager->initBLE();
    this->lokitAPI = new LokitAPI();
    this->mqttManager = new MqttManager();
    this->mqttManager->startLoop();
}

void DeviceManager::init(){
    Serial.println("Loading device config...");
    config->loadConfig();

    const bool wifiReady = config->isWifiReady();
    const bool lokitReady = config->isLokitReady();

    if (wifiReady) {
        WiFi.begin(config->getWifiSsid(), config->getWifiPass());
        Serial.println("Connecting to WiFi");

        int waits = 0;
        while(!WiFi.isConnected()){
            if(waits == 10)
            break;

            delay(1000);
            Serial.print('.');
            waits++;
        }
    }

    const bool wifiConnected = WiFi.isConnected();

    if(wifiReady && !wifiConnected) Serial.println("Couldn't connect to WiFi!");
    else if (wifiReady) {
        Serial.println("WiFi connected!");
        Serial.printf("IPV4: ");
        Serial.println(WiFi.localIP());
        Serial.printf("MAC: ");
        Serial.println(WiFi.macAddress());
        Serial.println();
    }

    if (lokitReady) {
        Serial.printf("Lokit server: %s (HTTP %d, MQTT %d)\n", config->getServerAddress().c_str(), config->getHttpPort(), config->getMqttPort());
        lokitAPI->init(config->getServerAddress(), config->getHttpPort(), config->getDeviceToken());
        mqttManager->init(config->getServerAddress(), config->getMqttPort(), config->getDeviceToken());
    }

    config->setDeviceStatus(wifiReady && lokitReady && wifiConnected ? DeviceStatus::IDLE : DeviceStatus::NOT_CONF);
    if(config->getDeviceStatus() == DeviceStatus::NOT_CONF) Serial.println("Device not fully configured! Start BLE provisioning and upload the configuration!");
}

void DeviceManager::setDeviceStatus(DeviceStatus status){
    config->setDeviceStatus(status);
}

DeviceStatus& DeviceManager::getDeviceStatus(){
    return config->getDeviceStatus();
}

LokitAPI* DeviceManager::getLokitAPI(){
    return lokitAPI;
}

void DeviceManager::startProv(){
    Serial.println("Starting BLE provisioning...");
    setDeviceStatus(DeviceStatus::IN_PROV);
    bluetoothManager->startProv();
}

void DeviceManager::stopProv(){
    Serial.println("Stopping BLE provisioning...");
    bluetoothManager->stopProv();
    init();
}

void DeviceManager::toggleProv(){
    if (getDeviceStatus() == DeviceStatus::IN_PROV) {
        stopProv();
    } else {
        startProv();
    }
}

void DeviceManager::requestProvToggle(){
    provToggleRequested = true;
}

void DeviceManager::processRequests(){
    if (!provToggleRequested) {
        return;
    }

    provToggleRequested = false;
    toggleProv();
}

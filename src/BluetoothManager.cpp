#include <BluetoothManager.hpp>
#include <PrefKeys.hpp>
#include <PrefCallback.hpp>

void BluetoothManager::initBLE(bool advertiseWifi, bool advertiseLokit){
    BLEDevice::init("LOKIT-READER");

    bleServer = BLEDevice::createServer();
    if (advertiseWifi) {
        bleWifiService = bleServer->createService(WIFI_SERVICE_UUID);

        bleWifiSsidChar = bleWifiService->createCharacteristic(
            WIFI_SSID_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
        );

        bleWifiPassChar = bleWifiService->createCharacteristic(
            WIFI_PASS_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
        );

        bleWifiSsidChar->setValue("SSID HERE");
        bleWifiPassChar->setValue("PASS HERE");

        bleWifiSsidChar->setCallbacks(new PrefCallback(prefs, WIFI_SSID_KEY));
        bleWifiPassChar->setCallbacks(new PrefCallback(prefs, WIFI_PASS_KEY));

        bleWifiService->start();
    }

    if (advertiseLokit) {
        bleLokitService = bleServer->createService(LOKIT_SERVICE_UUID);

        bleServerUrlChar = bleLokitService->createCharacteristic(
            LOKIT_SERVER_URL_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
        );

        bleTokenChar = bleLokitService->createCharacteristic(
            LOKIT_TOKEN_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
        );

        bleServerUrlChar->setValue("LOKIT SERVER URL HERE");
        bleTokenChar->setValue("LOKIT TOKEN HERE");

        bleServerUrlChar->setCallbacks(new PrefCallback(prefs, LOKIT_SERVER_KEY));
        bleTokenChar->setCallbacks(new PrefCallback(prefs, LOKIT_TOKEN_KEY));

        bleLokitService->start();
    }

    bleAdv = BLEDevice::getAdvertising();

    if (advertiseWifi) bleAdv->addServiceUUID(WIFI_SERVICE_UUID);
    if (advertiseLokit) bleAdv->addServiceUUID(LOKIT_SERVICE_UUID);

    bleAdv->setScanResponse(true);
    bleAdv->setMinPreferred(0x06);
    bleAdv->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}
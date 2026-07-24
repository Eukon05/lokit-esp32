#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Preferences.h>

#define WIFI_SERVICE_UUID "de2b0263-77bd-4c67-8221-f715595daa3c"
#define LOKIT_SERVICE_UUID "ada3cbf4-cc7d-4a35-b1bf-376dd44caff6"
#define WIFI_SSID_CHARACTERISTIC_UUID "7fde7291-c168-44a1-b70a-6c555fc7080b"
#define WIFI_PASS_CHARACTERISTIC_UUID "fcd9d714-5194-4011-abb9-6c0a1690f465"
#define LOKIT_SERVER_URL_CHARACTERISTIC_UUID "f6c1a2a5-1c93-4c61-a4fd-8c501c7f6c4a"
#define LOKIT_TOKEN_CHARACTERISTIC_UUID "6d04e8d5-6c6d-4c29-bcd1-7f0d6c1bf6cb"

class BluetoothManager
{
private:
    BLEServer *bleServer;
    BLEService *bleWifiService;
    BLEService *bleLokitService;

    BLECharacteristic *bleWifiSsidChar;
    BLECharacteristic *bleWifiPassChar;

    BLECharacteristic *bleServerUrlChar;
    BLECharacteristic *bleTokenChar;

    BLEAdvertising *bleAdv;

    Preferences* prefs;

    bool provInProgress = false;

public:
    BluetoothManager(Preferences* preferences) : prefs(preferences) {}
    void initBLE();
    void startProv();
    void stopProv();
    bool isProvInProgress();
};

#include <Arduino.h>
#include <Preferences.h>
#include <BLEUtils.h>

class PrefCallback : public BLECharacteristicCallbacks {
    private:
        const char* prefName;
        Preferences* prefs;

    public:
        PrefCallback(Preferences* preferences, const char* preferenceName) : prefs(preferences), prefName(preferenceName) {}
        void onWrite(BLECharacteristic *pLedCharacteristic);
};
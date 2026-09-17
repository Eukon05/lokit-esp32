#include <Arduino.h>
#include <Preferences.h>
#include <BLEUtils.h>

class PrefCallback : public BLECharacteristicCallbacks {
    private:
        const char* prefName;
        const bool isInt;
        Preferences* prefs;

    public:
        PrefCallback(Preferences* preferences, const char* preferenceName, bool isInt = false): isInt(isInt), prefs(preferences), prefName(preferenceName) {}
        void onWrite(BLECharacteristic *pLedCharacteristic);
};
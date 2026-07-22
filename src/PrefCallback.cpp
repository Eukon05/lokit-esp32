#include <PrefCallback.hpp>

void PrefCallback::onWrite(BLECharacteristic *pLedCharacteristic)
{
    std::string value = pLedCharacteristic->getValue();
    if (!value.empty())
    {
        Serial.printf("CALLBACK FOR %s\n", prefName);
        Serial.printf("READ VAL: %s\n", value.c_str());

        prefs->putString(prefName, value.c_str());
    }
}
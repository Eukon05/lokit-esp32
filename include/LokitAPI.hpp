#include <Arduino.h>

class LokitAPI {
    private:
        String serverPath;
        String deviceToken;
    public:
        LokitAPI(String serverName, String deviceToken);
        bool requestDecision(String cardUid);
};
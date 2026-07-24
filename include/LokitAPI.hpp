#include <Arduino.h>

enum DecisionOutcome {
    OK, DENIED, CONN_ERR, TOKEN_REVOKED, UNKNOWN_CODE, MALFORMED_BODY
};

class LokitAPI {
    private:
        String serverPath;
        String deviceToken;
    public:
        LokitAPI(String serverName, String deviceToken);
        DecisionOutcome requestDecision(String cardUid);
};
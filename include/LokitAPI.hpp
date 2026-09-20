#pragma once

#include <Arduino.h>

enum DecisionOutcome {
    ACCESS_OK, ACCESS_DENIED, CONN_ERR, TOKEN_REVOKED, UNKNOWN_CODE, MALFORMED_BODY
};

class LokitAPI {
    private:
        String serverPath;
        String deviceToken;
    public:
        void init(String serverName, int serverPort, String deviceToken);
        DecisionOutcome requestDecision(String cardUid);
};
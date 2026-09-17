#include <LokitAPI.hpp>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#define HTTP_PREFIX "http://"
#define DECISION_PATH "/api/v1/decision?cardId="

void LokitAPI::init(String serverName, int serverPort, String deviceToken) {
    serverPath = serverName;
    serverPath.trim();

    if (!serverPath.startsWith(HTTP_PREFIX))
        serverPath = HTTP_PREFIX + serverPath;

    while (serverPath.endsWith("/"))
        serverPath.remove(serverPath.length() - 1);

    serverPath += ":" + String(serverPort) + DECISION_PATH;

    this->deviceToken = deviceToken;
}

DecisionOutcome LokitAPI::requestDecision(String cardUid) {
    String reqAddr = serverPath + cardUid;
    HTTPClient http;

    Serial.printf("Lokit API request: %s\n", reqAddr.c_str());

    if (!http.begin(reqAddr.c_str())) {
        return DecisionOutcome::CONN_ERR;
    }

    http.addHeader("Authorization", "Bearer " + deviceToken);

    int responseCode = http.GET();

    if (responseCode < 0) {
        http.end();
        return DecisionOutcome::CONN_ERR;
    }

    if (responseCode == 401 || responseCode == 403){
        Serial.println("!!!DEVICE TOKEN REVOKED!!!");
        http.end();
        return DecisionOutcome::TOKEN_REVOKED;
    }
    else if (responseCode == 200){
        String body = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);

        http.end();

        if (error) return DecisionOutcome::MALFORMED_BODY;

        bool decision = doc["decision"];
        return decision ? DecisionOutcome::ACCESS_OK : DecisionOutcome::ACCESS_DENIED;
    }
    else {
        http.end();
        return DecisionOutcome::UNKNOWN_CODE;
    }
}
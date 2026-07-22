#include <LokitAPI.hpp>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#define HTTP_PREFIX "http://"
#define DECISION_PATH "/api/v1/decision?cardId="

LokitAPI::LokitAPI(String serverName, String deviceToken) {
    serverPath = serverName;

    if (!serverPath.startsWith(HTTP_PREFIX))
        serverPath = HTTP_PREFIX + serverPath;

    if (!serverPath.endsWith(DECISION_PATH))
        serverPath += DECISION_PATH;

    this->deviceToken = deviceToken;
}

bool LokitAPI::requestDecision(String cardUid) {
    String reqAddr = serverPath + cardUid;
    HTTPClient http;

    http.begin(reqAddr.c_str());
    http.addHeader("Authorization", "Bearer " + deviceToken);

    int responseCode = http.GET();

    if (responseCode == 401 || responseCode == 403){
        Serial.println("!!!DEVICE TOKEN REVOKED!!!");
        return false;
    }
    else{
        String body = http.getString();
        JsonDocument doc;
        deserializeJson(doc, body);

        bool decision = doc["decision"];
        return decision;
    }

    http.end();
}
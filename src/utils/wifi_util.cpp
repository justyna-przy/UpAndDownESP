#include "wifi_util.h"
#include <WiFi.h>

void wifi_connect(const char* ssid, const char* password)
{
    Serial.printf("[WiFi] Connecting to '%s'...\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Attempting to connect to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n[WiFi] Connected.");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
}

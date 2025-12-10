#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- CONFIG ---
// TODO: put real WiFi credentials here
const char* WIFI_SSID     = "Bingus Network";
const char* WIFI_PASSWORD = "xirl7972";

// Mark's server
const char* MQTT_HOST = "alderaan.software-engineering.ie";
const uint16_t MQTT_PORT = 1883;

// Topics for testing
const char* MQTT_TOPIC_SUB   = "test/justyna/sub";
const char* MQTT_TOPIC_PUB   = "test/justyna/pub";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectWiFi();
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n[ESP32] MQTT test starting...");

    connectWiFi();

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    connectMQTT();

    // Send a hello message once
    mqttClient.publish(MQTT_TOPIC_PUB, "ESP32 online!");
}

void loop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop(); // handles incoming messages + keepalive

    // every 5 seconds, publish a heartbeat
    static unsigned long lastPub = 0;
    if (millis() - lastPub > 5000) {
        lastPub = millis();
        mqttClient.publish(MQTT_TOPIC_PUB, "heartbeat");
        Serial.println("[ESP32] Published heartbeat");
    }
}

/* ------------ WIFI + MQTT helpers ---------------- */

void connectWiFi() {
    Serial.printf("[ESP32] Connecting to WiFi SSID '%s'...\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[ESP32] WiFi connected.");
    Serial.print("[ESP32] IP: ");
    Serial.println(WiFi.localIP());
}

void connectMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("[ESP32] Connecting to MQTT...");
        String clientId = "esp32-justyna-";
        clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

        if (mqttClient.connect(clientId.c_str())) {
            Serial.println("connected.");

            // subscribe to a topic
            mqttClient.subscribe(MQTT_TOPIC_SUB);
            Serial.printf("[ESP32] Subscribed to '%s'\n", MQTT_TOPIC_SUB);
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("[ESP32] Message on ");
    Serial.print(topic);
    Serial.print(": ");

    // copy payload to a string for printing
    String msg;
    for (unsigned int i = 0; i < length; ++i) {
        msg += (char)payload[i];
    }
    Serial.println(msg);

    // optional: echo it back on the pub topic
    mqttClient.publish(MQTT_TOPIC_PUB, msg.c_str());
}

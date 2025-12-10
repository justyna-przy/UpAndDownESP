#include "mqtt_util.h"
#include <WiFi.h>
#include <PubSubClient.h>

static WiFiClient s_wifiClient;
static PubSubClient s_mqttClient(s_wifiClient);
static String s_topicEvents;
static String s_topicCmd;
static MqttCommandHandler s_cmdHandler = nullptr;

static void mqtt_connect();

// Internal callback for PubSubClient
static void mqtt_internal_callback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    msg.reserve(length);
    for (unsigned int i = 0; i < length; ++i)
    {
        msg += (char)payload[i];
    }

    Serial.print("[MQTT] Message on ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(msg);

    // Only treat messages on the command topic as commands
    if (String(topic) == s_topicCmd && s_cmdHandler)
    {
        s_cmdHandler(msg);
    }
}

void mqtt_begin(const char *host,
                uint16_t port,
                const char *topicEvents,
                const char *topicCmd,
                MqttCommandHandler handler)
{
    s_topicEvents = topicEvents;
    s_topicCmd = topicCmd;
    s_cmdHandler = handler;

    s_mqttClient.setServer(host, port);
    s_mqttClient.setCallback(mqtt_internal_callback);

    mqtt_connect();
}

static void mqtt_connect()
{
    while (!s_mqttClient.connected())
    {
        Serial.print("[MQTT] Connecting... ");

        String clientId = "esp32-lift-";
        clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

        if (s_mqttClient.connect(clientId.c_str()))
        {
            Serial.println("connected.");
            if (!s_topicCmd.isEmpty())
            {
                s_mqttClient.subscribe(s_topicCmd.c_str());
                Serial.printf("[MQTT] Subscribed to '%s'\n", s_topicCmd.c_str());
            }
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(s_mqttClient.state());
            Serial.println(" retry in 2s");
            delay(2000);
        }
    }
}

void mqtt_loop()
{
    if (!s_mqttClient.connected())
    {
        mqtt_connect();
    }
    s_mqttClient.loop();
}

bool mqtt_publish_event(const String &payload)
{
    if (s_topicEvents.isEmpty())
    {
        Serial.println("[MQTT] ERROR: events topic not set");
        return false;
    }

    bool ok = s_mqttClient.publish(s_topicEvents.c_str(), payload.c_str());
    if (!ok)
    {
        Serial.println("[MQTT] Publish failed");
    }
    else
    {
        Serial.print("[MQTT] Published event: ");
        Serial.println(payload);
    }
    return ok;
}

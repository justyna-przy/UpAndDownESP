#include <Arduino.h>

#include "config.h"
#include "utils/wifi_util.h"
#include "utils/mqtt_util.h"

static void handle_mqtt_command(const String& payload)
{
    Serial.print("[APP] Command from MQTT: ");
    Serial.println(payload);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Connect to WiFi before starting MQTT
    wifi_connect(WIFI_SSID, WIFI_PASSWORD);

    mqtt_begin(
        MQTT_HOST,
        MQTT_PORT,
        MQTT_TOPIC_EVENTS,
        MQTT_TOPIC_CMD,
        handle_mqtt_command
    );
}

void loop()
{
    mqtt_loop();
    delay(5);
}

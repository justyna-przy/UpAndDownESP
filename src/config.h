#pragma once
#include <cstdint>

// WiFi credentials
static const char *WIFI_SSID = "Grove Island";
static const char *WIFI_PASSWORD = "island1234";

// MQTT Broker settings
static const char *MQTT_HOST = "alderaan.software-engineering.ie";
static const uint16_t MQTT_PORT = 1883;

// MQTT Topics
static const char *MQTT_TOPIC_EVENTS = "lift/justyna/events";
static const char *MQTT_TOPIC_CMD = "lift/justyna/cmd";

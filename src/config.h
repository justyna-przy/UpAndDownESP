#pragma once
#include <cstdint>

// WiFi credentials
static const char *WIFI_SSID = "Bingus Network";
static const char *WIFI_PASSWORD = "xirl7972";

// MQTT Broker settings
static const char *MQTT_HOST = "alderaan.software-engineering.ie";
static const uint16_t MQTT_PORT = 1883;

// MQTT Topics
static const char *MQTT_TOPIC_EVENTS = "lift/justyna/events";
static const char *MQTT_TOPIC_CMD = "lift/justyna/cmd";

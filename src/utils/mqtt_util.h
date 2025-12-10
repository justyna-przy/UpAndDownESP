#pragma once
#include <Arduino.h>

// Called when a command is received from MQTT (payload is raw string)
typedef void (*MqttCommandHandler)(const String &payload);

void mqtt_begin(const char *host,
                uint16_t port,
                const char *topicEvents,
                const char *topicCmd,
                MqttCommandHandler handler);

void mqtt_loop();

bool mqtt_publish_event(const String &payload);

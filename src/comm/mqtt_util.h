#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Callback type for receiving MQTT commands
typedef void (*mqtt_cmd_handler_t)(const char *payload, int len);

// Initialize MQTT client and connect to broker
void mqtt_init(const char *host, uint16_t port,
               const char *topic_events, const char *topic_cmd,
               mqtt_cmd_handler_t cmd_handler);

// Publish an event message
bool mqtt_publish_event(const char *payload);

// Check if MQTT is connected
bool mqtt_is_connected(void);

#ifdef __cplusplus
}
#endif

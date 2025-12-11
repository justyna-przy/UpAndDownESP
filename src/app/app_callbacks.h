#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// TinyFrame callbacks - heartbeat (ESP32 sends, MAX32655 responds)
void on_heartbeat_response(const uint8_t *data, uint16_t len);
void on_heartbeat_timeout(void);

// TinyFrame callback - data
void on_data_received(const uint8_t *data, uint16_t len);

// MQTT command callback
void on_mqtt_command(const char *payload, int len);

#ifdef __cplusplus
}
#endif

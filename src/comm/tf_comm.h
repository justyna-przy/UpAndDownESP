#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Message types
#define MSG_TYPE_HEARTBEAT   0x01
#define MSG_TYPE_DATA        0x02
#define MSG_TYPE_ESTOP       0x03
#define MSG_TYPE_CMD         0x04

// Callbacks for application layer
// WARNING: These callbacks are invoked while holding an internal mutex.
// Do NOT call tf_comm_send_*() from within callbacks - this will deadlock.
// If you need to send a response, queue the data and send from another context.
typedef void (*tf_heartbeat_response_cb)(const uint8_t *data, uint16_t len);
typedef void (*tf_heartbeat_timeout_cb)(void);
typedef void (*tf_data_received_cb)(const uint8_t *data, uint16_t len);

// Configuration
typedef struct {
    // Heartbeat callbacks (ESP32 sends, MAX32655 responds)
    tf_heartbeat_response_cb on_heartbeat_response;  // MAX32655 responded
    tf_heartbeat_timeout_cb on_heartbeat_timeout;    // MAX32655 didn't respond

    // Data callback
    tf_data_received_cb on_data_received;

    // Timing
    uint32_t heartbeat_interval_ms;   // How often to send heartbeats
    uint16_t heartbeat_timeout_ticks; // How long to wait for response
} tf_comm_config_t;

// Initialize TinyFrame communication (starts background task)
void tf_comm_init(const tf_comm_config_t *config);

// Send data (no response expected)
bool tf_comm_send_data(const uint8_t *data, uint16_t len);

// Send e-stop (high priority on receiver)
bool tf_comm_send_estop(const uint8_t *data, uint16_t len);

// Send command (low priority on receiver)
bool tf_comm_send_cmd(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

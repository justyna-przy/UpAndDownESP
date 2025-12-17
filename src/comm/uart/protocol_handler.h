#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"
#include "TinyFrame.h"

// === Callbacks (app layer implements) ===

// Command response received from MAX32655
typedef void (*protocol_cmd_response_cb)(const cmd_response_t *resp);
typedef void (*protocol_cmd_timeout_cb)(void);

// State event received from MAX32655
typedef void (*protocol_state_event_cb)(const state_event_t *evt);

// Heartbeat events
typedef void (*protocol_heartbeat_cb)(const uint8_t *data, uint16_t len);
typedef void (*protocol_heartbeat_timeout_cb)(void);

// === Configuration ===
typedef struct {
    protocol_cmd_response_cb on_cmd_response;
    protocol_cmd_timeout_cb on_cmd_timeout;
    protocol_state_event_cb on_state_event;
    protocol_heartbeat_cb on_heartbeat;
    protocol_heartbeat_timeout_cb on_heartbeat_timeout;
    uint32_t heartbeat_interval_ms;   // How often to send heartbeats
    uint16_t heartbeat_timeout_ticks; // TinyFrame ticks to wait for response
    uint16_t cmd_timeout_ticks;       // TinyFrame ticks to wait for cmd response
} protocol_config_t;

// === Init ===
void protocol_init(const protocol_config_t *config);

// === Sending (called by app layer) ===

// Send command to MAX32655 (response comes via on_cmd_response callback)
bool protocol_send_cmd(const cmd_request_t *cmd);

// Send e-stop to MAX32655
bool protocol_send_estop(const uint8_t *data, uint16_t len);

#endif // PROTOCOL_HANDLER_H

// MAX32655 communication - application layer

#include "app/max_comm.h"
#include "comm/uart/tf_transport.h"
#include "comm/uart/protocol_handler.h"
#include "comm/mqtt_util.h"
#include "protocol.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "MAX_COMM";

// Track connection state
static bool max32655_connected = false;

// === Protocol callbacks ===

static const char *direction_str(uint8_t dir)
{
    switch (dir) {
        case 0: return "stopped";
        case 1: return "up";
        case 2: return "down";
        default: return "unknown";
    }
}

static void on_cmd_response(const cmd_response_t *resp)
{
    ESP_LOGI(TAG, "CMD response: cmd=%d status=%d data_len=%d", resp->cmd_id, resp->status, resp->data_len);

    if (resp->status != CMD_OK) {
        char msg[32];
        snprintf(msg, sizeof(msg), "cmd_err_%d", resp->status);
        mqtt_publish_event(msg);
        return;
    }

    // Parse response data based on command type
    if (resp->cmd_id == CMD_GET_STATUS && resp->data_len >= 3) {
        uint8_t floor = resp->data[0];
        uint8_t direction = resp->data[1];
        uint8_t dest_bitmask = resp->data[2];

        char msg[64];
        snprintf(msg, sizeof(msg), "status:floor=%d,dir=%s,dest=0x%02X",
                 floor, direction_str(direction), dest_bitmask);
        mqtt_publish_event(msg);
    } else {
        mqtt_publish_event("cmd_ok");
    }
}

static void on_cmd_timeout(void)
{
    ESP_LOGW(TAG, "CMD timeout - no response from MAX32655");
    mqtt_publish_event("cmd_timeout");
}

static void on_state_event(const state_event_t *evt)
{
    ESP_LOGI(TAG, "State event: type=%d data=%d", evt->event_type, evt->data);

    char msg[64];

    switch (evt->event_type) {
        case PROTO_EVT_STOPPED_AT_FLOOR:
            snprintf(msg, sizeof(msg), "stopped_at_floor_%d", evt->data);
            break;

        case PROTO_EVT_CABIN_BUTTON:
            snprintf(msg, sizeof(msg), "cabin_button_%d", evt->data);
            break;

        case PROTO_EVT_CALL_BUTTON:
            snprintf(msg, sizeof(msg), "call_button_%d", evt->data);
            break;

        case PROTO_EVT_ESTOP_ACTIVATED:
            snprintf(msg, sizeof(msg), "estop_activated");
            break;

        case PROTO_EVT_ESTOP_RELEASED:
            snprintf(msg, sizeof(msg), "estop_released");
            break;

        default:
            snprintf(msg, sizeof(msg), "unknown_event_%d", evt->event_type);
            break;
    }

    mqtt_publish_event(msg);
}

static void on_heartbeat(const uint8_t *data, uint16_t len)
{
    ESP_LOGI(TAG, "MAX32655 responded: %.*s", len, data);

    if (!max32655_connected) {
        max32655_connected = true;
        ESP_LOGI(TAG, "MAX32655 connected!");
        mqtt_publish_event("max32655_connected");
    }
}

static void on_heartbeat_timeout(void)
{
    ESP_LOGW(TAG, "MAX32655 heartbeat timeout!");

    if (max32655_connected) {
        max32655_connected = false;
        mqtt_publish_event("max32655_disconnected");
    }
}

// === Public API ===

void MaxComm_Init(void)
{
    // Initialize transport layer
    tf_transport_init();

    // Initialize protocol layer
    protocol_config_t proto_cfg = {
        .on_cmd_response = on_cmd_response,
        .on_cmd_timeout = on_cmd_timeout,
        .on_state_event = on_state_event,
        .on_heartbeat = on_heartbeat,
        .on_heartbeat_timeout = on_heartbeat_timeout,
        .heartbeat_interval_ms = 10000,
        .heartbeat_timeout_ticks = 500,
        .cmd_timeout_ticks = 500,
    };
    protocol_init(&proto_cfg);

    ESP_LOGI(TAG, "MAX32655 communication initialized");
}

bool MaxComm_SendCmd(const cmd_request_t *cmd)
{
    return protocol_send_cmd(cmd);
}

bool MaxComm_SendEstop(void)
{
    uint8_t data[] = { 0x01 };  // Simple estop signal
    return protocol_send_estop(data, sizeof(data));
}

bool MaxComm_SendMoveToFloor(uint8_t floor)
{
    cmd_request_t cmd = {
        .cmd_id = CMD_MOVE_TO_FLOOR,
        .params_len = 1,
    };
    cmd.params[0] = floor;
    return protocol_send_cmd(&cmd);
}

bool MaxComm_SendGetStatus(void)
{
    cmd_request_t cmd = {
        .cmd_id = CMD_GET_STATUS,
        .params_len = 0,
    };
    return protocol_send_cmd(&cmd);
}

bool MaxComm_SendReset(void)
{
    cmd_request_t cmd = {
        .cmd_id = CMD_RESET,
        .params_len = 0,
    };
    return protocol_send_cmd(&cmd);
}

// Parse simple commands from MQTT
// Format: "CMD_NAME" or "CMD_NAME:param"
void MaxComm_OnMqttCommand(const char *payload, int len)
{
    ESP_LOGI(TAG, "MQTT cmd: %.*s", len, payload);

    // Simple string matching for now
    if (strncmp(payload, "status", len) == 0 || strncmp(payload, "STATUS", len) == 0) {
        MaxComm_SendGetStatus();
    }
    else if (strncmp(payload, "reset", len) == 0 || strncmp(payload, "RESET", len) == 0) {
        MaxComm_SendReset();
    }
    else if (strncmp(payload, "estop", len) == 0 || strncmp(payload, "ESTOP", len) == 0) {
        MaxComm_SendEstop();
    }
    else if (strncmp(payload, "floor:", 6) == 0 && len > 6) {
        // Parse floor number: "floor:2"
        int floor = atoi(payload + 6);
        MaxComm_SendMoveToFloor((uint8_t)floor);
    }
    else {
        ESP_LOGW(TAG, "Unknown MQTT command: %.*s", len, payload);
    }
}

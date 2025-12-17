#include "comm/protocol_handler.h"
#include "comm/tf_transport.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

// Configuration
static protocol_config_t proto_config;

// Heartbeat counter
static uint32_t heartbeat_counter = 0;

// Task handle
static TaskHandle_t protocol_task_handle = NULL;

// === Heartbeat handling ===

static TF_Result heartbeat_response_listener(TinyFrame *tf, TF_Msg *msg)
{
    (void)tf;

    printf("[PROTO] RX HB response: %.*s\n", msg->len, (const char *)msg->data);

    if (proto_config.on_heartbeat && msg->data) {
        proto_config.on_heartbeat(msg->data, msg->len);
    }

    return TF_CLOSE;  // One-shot listener
}

static TF_Result heartbeat_timeout_listener(TinyFrame *tf)
{
    (void)tf;

    printf("[PROTO] HB timeout!\n");

    if (proto_config.on_heartbeat_timeout) {
        proto_config.on_heartbeat_timeout();
    }

    return TF_CLOSE;
}

static void send_heartbeat(void)
{
    char msg[32];
    int len = snprintf(msg, sizeof(msg), "HB %lu", (unsigned long)heartbeat_counter++);

    printf("[PROTO] TX HB: %s\n", msg);

    tf_transport_query(MSG_TYPE_HEARTBEAT, (const uint8_t *)msg, len,
                       heartbeat_response_listener,
                       heartbeat_timeout_listener,
                       proto_config.heartbeat_timeout_ticks);
}

// === Command response handling ===

static TF_Result cmd_response_listener(TinyFrame *tf, TF_Msg *msg)
{
    (void)tf;

    if (msg->len < sizeof(cmd_response_t) || !msg->data) {
        printf("[PROTO] Invalid CMD response len=%d\n", msg->len);
        return TF_CLOSE;
    }

    const cmd_response_t *resp = (const cmd_response_t *)msg->data;

    printf("[PROTO] RX CMD response status=%d\n", resp->status);

    if (proto_config.on_cmd_response) {
        proto_config.on_cmd_response(resp);
    }

    return TF_CLOSE;  // One-shot listener
}

static TF_Result cmd_timeout_listener(TinyFrame *tf)
{
    (void)tf;

    printf("[PROTO] CMD timeout!\n");

    if (proto_config.on_cmd_timeout) {
        proto_config.on_cmd_timeout();
    }

    return TF_CLOSE;
}

// === Event handling ===

static TF_Result event_listener(TinyFrame *tf, TF_Msg *msg)
{
    (void)tf;

    if (msg->len < sizeof(state_event_t) || !msg->data) {
        printf("[PROTO] Invalid EVENT len=%d\n", msg->len);
        return TF_STAY;
    }

    const state_event_t *evt = (const state_event_t *)msg->data;

    printf("[PROTO] RX EVENT type=%d data=%d\n", evt->event_type, evt->data);

    if (proto_config.on_state_event) {
        proto_config.on_state_event(evt);
    }

    return TF_STAY;  // Keep listening
}

// === Protocol task (heartbeat timing) ===

static void protocol_task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_heartbeat_time = xTaskGetTickCount();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));  // Check every 100ms

        if (proto_config.heartbeat_interval_ms > 0) {
            TickType_t now = xTaskGetTickCount();
            if ((now - last_heartbeat_time) >= pdMS_TO_TICKS(proto_config.heartbeat_interval_ms)) {
                send_heartbeat();
                last_heartbeat_time = now;
            }
        }
    }
}

void protocol_init(const protocol_config_t *config)
{
    if (config) {
        proto_config = *config;
    }

    // Register listeners with transport layer
    tf_transport_add_listener(MSG_TYPE_EVENT, event_listener);

    printf("[PROTO] Protocol handler init (HB interval=%lums, timeout=%d ticks)\n",
           (unsigned long)proto_config.heartbeat_interval_ms,
           proto_config.heartbeat_timeout_ticks);

    // Create protocol task for heartbeat timing
    xTaskCreate(protocol_task, "protocol", 4096, NULL, 4, &protocol_task_handle);
}

bool protocol_send_cmd(const cmd_request_t *cmd)
{
    printf("[PROTO] TX CMD id=%d params_len=%d\n", cmd->cmd_id, cmd->params_len);

    return tf_transport_query(MSG_TYPE_CMD, (const uint8_t *)cmd, sizeof(cmd_request_t),
                              cmd_response_listener,
                              cmd_timeout_listener,
                              proto_config.cmd_timeout_ticks);
}

bool protocol_send_estop(const uint8_t *data, uint16_t len)
{
    printf("[PROTO] TX E-STOP\n");
    return tf_transport_send(MSG_TYPE_ESTOP, data, len);
}

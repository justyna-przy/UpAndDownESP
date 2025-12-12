// Application callbacks for TinyFrame and MQTT

#include "app/app_callbacks.h"
#include "comm/tf_comm.h"
#include "comm/mqtt_util.h"
#include "esp_log.h"
#include <stdbool.h>

static const char *TAG = "APP";

// Track connection state for first-heartbeat detection
static bool max32655_connected = false;

// TinyFrame callbacks - heartbeat (ESP32 sends, MAX32655 responds)
void on_heartbeat_response(const uint8_t *data, uint16_t len)
{
    ESP_LOGI(TAG, "MAX32655 responded: %.*s", len, data);

    // First successful heartbeat = connection established
    if (!max32655_connected) {
        max32655_connected = true;
        ESP_LOGI(TAG, "MAX32655 connected!");
        mqtt_publish_event("MAX32655 connected");
    }
}

void on_heartbeat_timeout(void)
{
    ESP_LOGW(TAG, "MAX32655 heartbeat timeout - connection lost!");

    // Only publish if we were previously connected
    if (max32655_connected) {
        max32655_connected = false;
        mqtt_publish_event("heartbeat timeout, connection fail?");
    }
}

void on_data_received(const uint8_t *data, uint16_t len)
{
    ESP_LOGI(TAG, "Data [%d bytes]", len);
    ESP_LOG_BUFFER_HEX(TAG, data, len);
    // Bridge to MQTT if needed: mqtt_publish_event(...)
}

// MQTT command callback - forward commands to MAX32655 via TinyFrame
void on_mqtt_command(const char *payload, int len)
{
    ESP_LOGI(TAG, "MQTT cmd: %.*s", len, payload);
    // Forward to MAX32655 via TinyFrame CMD message
    tf_comm_send_cmd((const uint8_t *)payload, len);
}

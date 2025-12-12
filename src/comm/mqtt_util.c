// ESP-IDF MQTT client utility

#include "comm/mqtt_util.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static const char *s_topic_events = NULL;
static const char *s_topic_cmd = NULL;
static mqtt_cmd_handler_t s_cmd_handler = NULL;
static bool s_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Connected to broker");
            s_connected = true;
            // Subscribe to command topic
            if (s_topic_cmd) {
                int msg_id = esp_mqtt_client_subscribe(s_mqtt_client, s_topic_cmd, 1);
                ESP_LOGI(TAG, "Subscribed to '%s' (msg_id=%d)", s_topic_cmd, msg_id);
            }
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Disconnected from broker");
            s_connected = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Subscribed (msg_id=%d)", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Message on topic '%.*s'", event->topic_len, event->topic);
            // Check if this is our command topic
            if (s_cmd_handler && s_topic_cmd &&
                event->topic_len == (int)strlen(s_topic_cmd) &&
                strncmp(event->topic, s_topic_cmd, event->topic_len) == 0) {
                s_cmd_handler(event->data, event->data_len);
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;

        default:
            break;
    }
}

void mqtt_init(const char *host, uint16_t port,
               const char *topic_events, const char *topic_cmd,
               mqtt_cmd_handler_t cmd_handler)
{
    s_topic_events = topic_events;
    s_topic_cmd = topic_cmd;
    s_cmd_handler = cmd_handler;

    // Build broker URI
    char uri[128];
    snprintf(uri, sizeof(uri), "mqtt://%s:%d", host, port);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = uri,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return;
    }

    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_mqtt_client);

    ESP_LOGI(TAG, "Connecting to %s", uri);
}

bool mqtt_publish_event(const char *payload)
{
    if (!s_connected || !s_mqtt_client || !s_topic_events) {
        ESP_LOGW(TAG, "Cannot publish - not connected or topic not set");
        return false;
    }

    int msg_id = esp_mqtt_client_publish(s_mqtt_client, s_topic_events, payload, 0, 1, 0);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Publish failed");
        return false;
    }

    ESP_LOGI(TAG, "Published event (msg_id=%d): %s", msg_id, payload);
    return true;
}

bool mqtt_is_connected(void)
{
    return s_connected;
}

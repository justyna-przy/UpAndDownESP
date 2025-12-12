// ESP-IDF main for UpAndDownESP
// Connects WiFi, MQTT, and TinyFrame communication with MAX32655

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "config.h"
#include "comm/wifi_util.h"
#include "comm/mqtt_util.h"
#include "comm/tf_comm.h"
#include "app/app_callbacks.h"

static const char *TAG = "MAIN";

extern "C" void app_main(void)
{
    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Starting UpAndDownESP...");

    // Connect to WiFi (blocking)
    wifi_init(WIFI_SSID, WIFI_PASSWORD);

    // Initialize MQTT
    mqtt_init(MQTT_HOST, MQTT_PORT, MQTT_TOPIC_EVENTS, MQTT_TOPIC_CMD, on_mqtt_command);

    // Initialize TinyFrame communication (creates background FreeRTOS task)
    tf_comm_config_t tf_config = {};
    tf_config.on_heartbeat_response = on_heartbeat_response;
    tf_config.on_heartbeat_timeout = on_heartbeat_timeout;
    tf_config.on_data_received = on_data_received;
    tf_config.heartbeat_interval_ms = 10000;  // 10 second heartbeat
    tf_config.heartbeat_timeout_ticks = 500;
    tf_comm_init(&tf_config);

    ESP_LOGI(TAG, "Setup complete");

    // Main task can exit - all work happens in FreeRTOS tasks
}

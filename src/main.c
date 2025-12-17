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
#include "app/max_comm.h"

static const char *TAG = "MAIN";

void app_main(void)
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

    // Initialize MAX32655 communication (transport + protocol layers)
    MaxComm_Init();

    // Initialize MQTT (uses MaxComm_OnMqttCommand callback)
    mqtt_init(MQTT_HOST, MQTT_PORT, MQTT_TOPIC_EVENTS, MQTT_TOPIC_CMD, MaxComm_OnMqttCommand);

    ESP_LOGI(TAG, "Setup complete");

    // Main task can exit - all work happens in FreeRTOS tasks
}

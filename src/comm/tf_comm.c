#include "comm/tf_comm.h"
#include "TinyFrame.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>

// UART configuration
#define UART_NUM_MAX        UART_NUM_2
#define MAX_RX_PIN          25
#define MAX_TX_PIN          26
#define UART_BAUD           115200
#define UART_BUF_SIZE       256

// Task configuration
#define TF_TASK_STACK_SIZE  4096
#define TF_TASK_PRIORITY    5

// TinyFrame instance
static TinyFrame tf_instance;
static TinyFrame *tf = &tf_instance;

// Application callbacks and config
static tf_comm_config_t app_config;

// Mutex for TinyFrame access
static SemaphoreHandle_t tf_mutex;

// Heartbeat counter
static uint32_t heartbeat_counter = 0;

// Task handle
static TaskHandle_t tf_task_handle = NULL;

// UART write implementation for TinyFrame
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    (void)tf;
    uart_write_bytes(UART_NUM_MAX, buff, len);
}

// Internal listener for heartbeat responses from MAX32655
static TF_Result heartbeat_response_listener(TinyFrame *tf, TF_Msg *msg)
{
    (void)tf;
    if (app_config.on_heartbeat_response && msg->data) {
        app_config.on_heartbeat_response(msg->data, msg->len);
    }
    return TF_CLOSE;
}

// Internal timeout callback for heartbeat (MAX32655 didn't respond)
static TF_Result heartbeat_timeout_listener(TinyFrame *tf)
{
    (void)tf;
    if (app_config.on_heartbeat_timeout) {
        app_config.on_heartbeat_timeout();
    }
    return TF_CLOSE;
}

// Internal listener for incoming data messages
static TF_Result data_listener(TinyFrame *tf, TF_Msg *msg)
{
    (void)tf;
    if (app_config.on_data_received && msg->data) {
        app_config.on_data_received(msg->data, msg->len);
    }
    return TF_STAY;
}

// Generic fallback listener
static TF_Result generic_listener(TinyFrame *tf, TF_Msg *msg)
{
    (void)tf;
    printf("[TF] Unknown msg type=%d id=%d len=%d\n", msg->type, msg->frame_id, msg->len);
    return TF_STAY;
}

static void uart_init_internal(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_MAX, UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_MAX, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_MAX, MAX_TX_PIN, MAX_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

static void send_heartbeat(void)
{
    char msg[32];
    int len = snprintf(msg, sizeof(msg), "HB %lu", (unsigned long)heartbeat_counter++);

    printf("[TF] TX: %s\n", msg);

    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    TF_QuerySimple(tf, MSG_TYPE_HEARTBEAT, (const uint8_t *)msg, len,
                   heartbeat_response_listener,
                   heartbeat_timeout_listener,
                   app_config.heartbeat_timeout_ticks);
    xSemaphoreGive(tf_mutex);
}

// FreeRTOS task for TinyFrame communication
static void tf_comm_task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_tick_time = xTaskGetTickCount();
    TickType_t last_heartbeat_time = xTaskGetTickCount();

    while (true) {
        // Process RX
        uint8_t rx_buf[32];
        int len = uart_read_bytes(UART_NUM_MAX, rx_buf, sizeof(rx_buf), pdMS_TO_TICKS(10));

        xSemaphoreTake(tf_mutex, portMAX_DELAY);
        if (len > 0) {
            TF_Accept(tf, rx_buf, len);
        }

        // TF_Tick every ~10ms
        TickType_t now = xTaskGetTickCount();
        if ((now - last_tick_time) >= pdMS_TO_TICKS(10)) {
            TF_Tick(tf);
            last_tick_time = now;
        }
        xSemaphoreGive(tf_mutex);

        // Heartbeat
        if (app_config.heartbeat_interval_ms > 0) {
            if ((now - last_heartbeat_time) >= pdMS_TO_TICKS(app_config.heartbeat_interval_ms)) {
                send_heartbeat();
                last_heartbeat_time = now;
            }
        }
    }
}

void tf_comm_init(const tf_comm_config_t *config)
{
    if (config) {
        app_config = *config;
    }

    // Create mutex
    tf_mutex = xSemaphoreCreateMutex();
    configASSERT(tf_mutex);

    uart_init_internal();

    TF_InitStatic(tf, TF_MASTER);
    TF_AddTypeListener(tf, MSG_TYPE_DATA, data_listener);
    TF_AddGenericListener(tf, generic_listener);

    printf("[TF] Init (GPIO%d RX, GPIO%d TX @ %d baud)\n", MAX_RX_PIN, MAX_TX_PIN, UART_BAUD);

    // Create communication task
    xTaskCreate(tf_comm_task, "tf_comm", TF_TASK_STACK_SIZE, NULL, TF_TASK_PRIORITY, &tf_task_handle);
}

bool tf_comm_send_data(const uint8_t *data, uint16_t len)
{
    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    bool result = TF_SendSimple(tf, MSG_TYPE_DATA, data, len);
    xSemaphoreGive(tf_mutex);
    return result;
}

bool tf_comm_send_estop(const uint8_t *data, uint16_t len)
{
    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    bool result = TF_SendSimple(tf, MSG_TYPE_ESTOP, data, len);
    xSemaphoreGive(tf_mutex);
    return result;
}

bool tf_comm_send_cmd(const uint8_t *data, uint16_t len)
{
    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    bool result = TF_SendSimple(tf, MSG_TYPE_CMD, data, len);
    xSemaphoreGive(tf_mutex);
    return result;
}

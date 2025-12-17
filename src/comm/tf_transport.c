#include "comm/tf_transport.h"
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

// Mutex for TinyFrame access
static SemaphoreHandle_t tf_mutex;

// Task handle
static TaskHandle_t tf_task_handle = NULL;

// UART write implementation for TinyFrame
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    (void)tf;
    uart_write_bytes(UART_NUM_MAX, buff, len);
}

// Generic fallback listener
static TF_Result generic_listener(TinyFrame *tf, TF_Msg *msg)
{
    (void)tf;
    printf("[TF] Unhandled type=%d id=%d len=%d\n", msg->type, msg->frame_id, msg->len);
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

// FreeRTOS task for TinyFrame communication
static void tf_task(void *pvParameters)
{
    (void)pvParameters;

    while (true) {
        // Read available UART data
        uint8_t rx_buf[32];
        int len = uart_read_bytes(UART_NUM_MAX, rx_buf, sizeof(rx_buf), pdMS_TO_TICKS(10));

        xSemaphoreTake(tf_mutex, portMAX_DELAY);

        if (len > 0) {
            TF_Accept(tf, rx_buf, len);
        }

        // TinyFrame housekeeping
        TF_Tick(tf);

        xSemaphoreGive(tf_mutex);
    }
}

void tf_transport_init(void)
{
    // Create mutex
    tf_mutex = xSemaphoreCreateMutex();
    configASSERT(tf_mutex);

    uart_init_internal();

    TF_InitStatic(tf, TF_MASTER);
    TF_AddGenericListener(tf, generic_listener);

    printf("[TF] Transport init (GPIO%d RX, GPIO%d TX @ %d baud)\n", MAX_RX_PIN, MAX_TX_PIN, UART_BAUD);

    // Create communication task
    xTaskCreate(tf_task, "tf_transport", TF_TASK_STACK_SIZE, NULL, TF_TASK_PRIORITY, &tf_task_handle);
}

bool tf_transport_add_listener(uint8_t msg_type, tf_transport_listener_cb callback)
{
    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    bool result = TF_AddTypeListener(tf, msg_type, callback);
    xSemaphoreGive(tf_mutex);
    return result;
}

bool tf_transport_send(uint8_t msg_type, const uint8_t *data, uint16_t len)
{
    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    bool result = TF_SendSimple(tf, msg_type, data, len);
    xSemaphoreGive(tf_mutex);
    return result;
}

bool tf_transport_query(uint8_t msg_type, const uint8_t *data, uint16_t len,
                        tf_transport_listener_cb on_response,
                        tf_transport_timeout_cb on_timeout,
                        uint16_t timeout_ticks)
{
    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    bool result = TF_QuerySimple(tf, msg_type, data, len,
                                  on_response, on_timeout, timeout_ticks);
    xSemaphoreGive(tf_mutex);
    return result;
}

bool tf_transport_respond(TF_Msg *original_msg, const uint8_t *data, uint16_t len)
{
    original_msg->data = data;
    original_msg->len = (TF_LEN)len;

    xSemaphoreTake(tf_mutex, portMAX_DELAY);
    bool result = TF_Respond(tf, original_msg);
    xSemaphoreGive(tf_mutex);
    return result;
}

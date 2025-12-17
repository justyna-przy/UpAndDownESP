#ifndef TF_TRANSPORT_H
#define TF_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>
#include "TinyFrame.h"
#include "freertos/FreeRTOS.h"

// Message types (must match both sides)
#define MSG_TYPE_HEARTBEAT   0x01
#define MSG_TYPE_ESTOP       0x02
#define MSG_TYPE_CMD         0x03
#define MSG_TYPE_EVENT       0x04

// Callback types
typedef TF_Result (*tf_transport_listener_cb)(TinyFrame *tf, TF_Msg *msg);
typedef TF_Result (*tf_transport_timeout_cb)(TinyFrame *tf);

// Initialize transport layer (UART + TinyFrame + task)
void tf_transport_init(void);

// Register a listener for a specific message type
bool tf_transport_add_listener(uint8_t msg_type, tf_transport_listener_cb callback);

// Send message (blocking)
bool tf_transport_send(uint8_t msg_type, const uint8_t *data, uint16_t len);

// Send query expecting response (for heartbeat, commands)
bool tf_transport_query(uint8_t msg_type, const uint8_t *data, uint16_t len,
                        tf_transport_listener_cb on_response,
                        tf_transport_timeout_cb on_timeout,
                        uint16_t timeout_ticks);

// Respond to an incoming query (preserves frame_id)
bool tf_transport_respond(TF_Msg *original_msg, const uint8_t *data, uint16_t len);

#endif // TF_TRANSPORT_H

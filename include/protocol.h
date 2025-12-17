/**
 * @file protocol.h
 * @brief Shared communication protocol between ESP32 and MAX32655
 *
 * This header defines the message structures and enums used for
 * UART communication over TinyFrame between the two MCUs.
 *
 * Include this file in both projects to ensure consistent message formats.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// ============================================
// Command IDs (ESP -> Maxim)
// ============================================
typedef enum {
    CMD_NOP             = 0x00,
    CMD_GET_STATUS      = 0x01,
    CMD_MOVE_TO_FLOOR   = 0x02,
    CMD_RESET           = 0x03,
} cmd_id_t;

// ============================================
// Command Response Status (Maxim -> ESP)
// ============================================
typedef enum {
    CMD_OK              = 0x00,
    CMD_ERR_UNKNOWN     = 0x01,
    CMD_ERR_INVALID     = 0x02,
    CMD_ERR_BUSY        = 0x03,
} cmd_status_t;

// ============================================
// Protocol Event Types (Maxim -> ESP, unsolicited)
// Prefixed with PROTO_ to avoid collision with internal state machine events
// ============================================
typedef enum {
    PROTO_EVT_STOPPED_AT_FLOOR  = 0x01,  // data = floor number
    PROTO_EVT_CABIN_BUTTON      = 0x02,  // data = destination floor
    PROTO_EVT_CALL_BUTTON       = 0x03,  // data = call_button_t
    PROTO_EVT_ESTOP_ACTIVATED   = 0x04,  // data = unused
    PROTO_EVT_ESTOP_RELEASED    = 0x05,  // data = unused
} proto_event_type_t;

// ============================================
// Call Button IDs
// ============================================
typedef enum {
    CALL_BTN_0      = 0x00,  // ground floor
    CALL_BTN_1_DOWN = 0x01,
    CALL_BTN_1_UP   = 0x02,
    CALL_BTN_2      = 0x03,  // top floor
} call_button_t;

// ============================================
// Message Structs
// ============================================

/**
 * Command request (ESP -> Maxim)
 * Sent via TF_QuerySimple, expects cmd_response_t back
 */
typedef struct __attribute__((packed)) {
    uint8_t cmd_id;       // cmd_id_t
    uint8_t params[16];   // command-specific parameters
    uint8_t params_len;   // number of valid bytes in params
} cmd_request_t;

/**
 * Command response (Maxim -> ESP)
 * Sent via TF_Respond in reply to cmd_request_t
 */
typedef struct __attribute__((packed)) {
    uint8_t cmd_id;       // echo of cmd_id_t from request
    uint8_t status;       // cmd_status_t
    uint8_t data[16];     // response data (command-specific)
    uint8_t data_len;     // number of valid bytes in data
} cmd_response_t;

/**
 * State event (Maxim -> ESP, unsolicited)
 * Sent via TF_SendSimple when state changes occur
 */
typedef struct __attribute__((packed)) {
    uint8_t event_type;   // event_type_t
    uint8_t data;         // interpretation depends on event_type
} state_event_t;

#endif // PROTOCOL_H

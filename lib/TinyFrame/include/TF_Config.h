//
// TinyFrame configuration for ESP32 (FireBeetle)
//

#ifndef TF_CONFIG_H
#define TF_CONFIG_H

#include <stdint.h>
#include <stdio.h>

//----------------------------- FRAME FORMAT ---------------------------------
// ,-----+-----+-----+------+------------+- - - -+-------------,
// | SOF | ID  | LEN | TYPE | HEAD_CKSUM | DATA  | DATA_CKSUM  |
// | 0-1 | 1-4 | 1-4 | 1-4  | 0-4        | ...   | 0-4         | <- size (bytes)
// '-----+-----+-----+------+------------+- - - -+-------------'

// Field sizes (must match on both peers)
#define TF_ID_BYTES     1
#define TF_LEN_BYTES    2
#define TF_TYPE_BYTES   1

// Checksum type: CRC16 for reliable error detection
#define TF_CKSUM_TYPE TF_CKSUM_CRC16

// Use SOF byte to mark start of frame
#define TF_USE_SOF_BYTE 1
#define TF_SOF_BYTE     0x01

//----------------------- PLATFORM COMPATIBILITY ----------------------------

// Timeout tick counter type
typedef uint16_t TF_TICKS;

// Listener iteration counter type
typedef uint8_t TF_COUNT;

//----------------------------- PARAMETERS ----------------------------------

// Maximum received payload size
#define TF_MAX_PAYLOAD_RX 128

// Size of the sending buffer
#define TF_SENDBUF_LEN    64

// Listener slot counts
#define TF_MAX_ID_LST   4
#define TF_MAX_TYPE_LST 4
#define TF_MAX_GEN_LST  2

// Timeout for receiving & parsing a frame (ticks)
#define TF_PARSER_TIMEOUT_TICKS 50

// Disable mutex (we handle thread safety ourselves)
#define TF_USE_MUTEX  0

// Error reporting via ESP-IDF logging
#define TF_Error(format, ...) printf("[TF] " format "\n", ##__VA_ARGS__)

#endif // TF_CONFIG_H

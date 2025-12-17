#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize MAX32655 communication (transport + protocol layers)
void MaxComm_Init(void);

// Send command to MAX32655
bool MaxComm_SendCmd(const cmd_request_t *cmd);

// Send e-stop to MAX32655
bool MaxComm_SendEstop(void);

// Convenience command functions
bool MaxComm_SendMoveToFloor(uint8_t floor);
bool MaxComm_SendGetStatus(void);
bool MaxComm_SendReset(void);

// MQTT command callback (call this from MQTT handler)
void MaxComm_OnMqttCommand(const char *payload, int len);

#ifdef __cplusplus
}
#endif

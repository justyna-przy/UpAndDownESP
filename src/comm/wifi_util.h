#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize WiFi in station mode and connect (blocking until connected)
void wifi_init(const char *ssid, const char *password);

// Check if WiFi is connected
bool wifi_is_connected(void);

#ifdef __cplusplus
}
#endif

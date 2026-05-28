#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "esp_wifi_types_generic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CLAUDE_WIFI_MODE_NONE = 0,
    CLAUDE_WIFI_MODE_STATION,
    CLAUDE_WIFI_MODE_CONFIG_AP,
} claude_wifi_mode_t;

void claude_wifi_service_start(void);
void claude_wifi_service_enter_config_mode(void);
claude_wifi_mode_t claude_wifi_service_get_mode(void);
bool claude_wifi_service_is_connected(void);
const char *claude_wifi_service_get_config_ssid(void);
const char *claude_wifi_service_get_config_password(void);
const char *claude_wifi_service_get_connected_ssid(void);
const char *claude_wifi_service_get_station_ip(void);
esp_err_t claude_wifi_service_scan_networks(wifi_ap_record_t *records, uint16_t *count, uint16_t max_count);

#ifdef __cplusplus
}
#endif

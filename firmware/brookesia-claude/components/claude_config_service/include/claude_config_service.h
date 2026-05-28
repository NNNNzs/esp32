#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void claude_config_get_mqtt_broker_uri(char *buffer, size_t buffer_size);
void claude_config_get_mqtt_topic(char *buffer, size_t buffer_size);
void claude_config_get_mqtt_username(char *buffer, size_t buffer_size);
void claude_config_get_mqtt_password(char *buffer, size_t buffer_size);
void claude_config_get_wifi_ssid(char *buffer, size_t buffer_size);
void claude_config_get_wifi_password(char *buffer, size_t buffer_size);
bool claude_config_set_mqtt_broker_uri(const char *value);
bool claude_config_set_mqtt_topic(const char *value);
bool claude_config_set_mqtt_credentials(const char *username, const char *password);
bool claude_config_set_wifi_credentials(const char *ssid, const char *password);

#ifdef __cplusplus
}
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void claude_mqtt_service_start(void);
void claude_mqtt_service_stop(void);
bool claude_mqtt_service_is_running(void);
bool claude_mqtt_service_is_connected(void);

#ifdef __cplusplus
}
#endif

#include "claude_mqtt_service.h"

#include "claude_config_service.h"
#include "claude_status_service.h"
#include "mqtt_client.h"
#include "esp_log.h"

#include <string>

static const char *TAG = "ClaudeMQTT";
static esp_mqtt_client_handle_t mqtt_client = nullptr;
static char mqtt_topic[128] = {};
static char mqtt_broker_uri[128] = {};

static const char *STATUS_TOKENS[] = {
    "thinking",
    "working",
    "approval",
    "done",
    "error",
    "idle",
};

static void apply_payload(const char *data, int len)
{
    if (data == nullptr || len <= 0) {
        return;
    }

    std::string payload(data, len);

    for (const char *state : STATUS_TOKENS) {
        if (payload.find(state) != std::string::npos) {
            claude_status_service_set_state_from_text(state);
            return;
        }
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);

    switch (event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "connected, subscribing to %s", mqtt_topic);
        esp_mqtt_client_subscribe(mqtt_client, mqtt_topic, 1);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "disconnected");
        break;
    case MQTT_EVENT_DATA:
        apply_payload(event->data, event->data_len);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "mqtt error");
        claude_status_service_set_state(CLAUDE_STATUS_ERROR);
        break;
    default:
        break;
    }
}

void claude_mqtt_service_start(void)
{
    if (mqtt_client != nullptr) {
        return;
    }

    claude_config_get_mqtt_broker_uri(mqtt_broker_uri, sizeof(mqtt_broker_uri));
    claude_config_get_mqtt_topic(mqtt_topic, sizeof(mqtt_topic));

    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = mqtt_broker_uri;

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == nullptr) {
        ESP_LOGE(TAG, "failed to create mqtt client");
        claude_status_service_set_state(CLAUDE_STATUS_ERROR);
        return;
    }

    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, nullptr);
    ESP_LOGI(TAG, "starting mqtt client: %s", mqtt_broker_uri);
    esp_mqtt_client_start(mqtt_client);
}

#include "claude_config_service.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include <string.h>

static const char *NVS_NAMESPACE = "claude";
static const char *NVS_KEY_BROKER = "mqtt_broker";
static const char *NVS_KEY_TOPIC = "mqtt_topic";
static const char *NVS_KEY_MQTT_USERNAME = "mqtt_user";
static const char *NVS_KEY_MQTT_PASSWORD = "mqtt_pass";
static const char *NVS_KEY_WIFI_SSID = "wifi_ssid";
static const char *NVS_KEY_WIFI_PASS = "wifi_pass";

static void copy_default(char *buffer, size_t buffer_size, const char *value)
{
    if (buffer == nullptr || buffer_size == 0) {
        return;
    }
    strlcpy(buffer, value, buffer_size);
}

static bool read_string(const char *key, char *buffer, size_t buffer_size, const char *fallback)
{
    if (buffer == nullptr || buffer_size == 0) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        copy_default(buffer, buffer_size, fallback);
        return false;
    }

    size_t required_size = buffer_size;
    err = nvs_get_str(handle, key, buffer, &required_size);
    nvs_close(handle);

    if (err != ESP_OK) {
        copy_default(buffer, buffer_size, fallback);
        return false;
    }

    return true;
}

static bool write_string(const char *key, const char *value)
{
    if (value == nullptr || value[0] == '\0') {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return false;
    }

    err = nvs_set_str(handle, key, value);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);

    return err == ESP_OK;
}

void claude_config_get_mqtt_broker_uri(char *buffer, size_t buffer_size)
{
    read_string(NVS_KEY_BROKER, buffer, buffer_size, CONFIG_CLAUDE_MQTT_BROKER_URI);
}

void claude_config_get_mqtt_topic(char *buffer, size_t buffer_size)
{
    read_string(NVS_KEY_TOPIC, buffer, buffer_size, CONFIG_CLAUDE_MQTT_TOPIC);
}

void claude_config_get_mqtt_username(char *buffer, size_t buffer_size)
{
    read_string(NVS_KEY_MQTT_USERNAME, buffer, buffer_size, "");
}

void claude_config_get_mqtt_password(char *buffer, size_t buffer_size)
{
    read_string(NVS_KEY_MQTT_PASSWORD, buffer, buffer_size, "");
}

void claude_config_get_wifi_ssid(char *buffer, size_t buffer_size)
{
    read_string(NVS_KEY_WIFI_SSID, buffer, buffer_size, "");
}

void claude_config_get_wifi_password(char *buffer, size_t buffer_size)
{
    read_string(NVS_KEY_WIFI_PASS, buffer, buffer_size, "");
}

bool claude_config_set_mqtt_broker_uri(const char *value)
{
    return write_string(NVS_KEY_BROKER, value);
}

bool claude_config_set_mqtt_topic(const char *value)
{
    return write_string(NVS_KEY_TOPIC, value);
}

bool claude_config_set_mqtt_credentials(const char *username, const char *password)
{
    if (username == nullptr || password == nullptr) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return false;
    }

    if (username[0] != '\0') {
        err = nvs_set_str(handle, NVS_KEY_MQTT_USERNAME, username);
    } else {
        err = nvs_erase_key(handle, NVS_KEY_MQTT_USERNAME);
    }
    if (err == ESP_OK) {
        if (password[0] != '\0') {
            err = nvs_set_str(handle, NVS_KEY_MQTT_PASSWORD, password);
        } else {
            err = nvs_erase_key(handle, NVS_KEY_MQTT_PASSWORD);
        }
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);

    return err == ESP_OK;
}

bool claude_config_set_wifi_credentials(const char *ssid, const char *password)
{
    if (ssid == nullptr || ssid[0] == '\0' || password == nullptr) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return false;
    }

    err = nvs_set_str(handle, NVS_KEY_WIFI_SSID, ssid);
    if (err == ESP_OK) {
        err = nvs_set_str(handle, NVS_KEY_WIFI_PASS, password);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);

    return err == ESP_OK;
}

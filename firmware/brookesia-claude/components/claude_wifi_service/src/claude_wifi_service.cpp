#include "claude_wifi_service.h"

#include "claude_config_service.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "ClaudeWiFi";
static claude_wifi_mode_t current_mode = CLAUDE_WIFI_MODE_NONE;
static bool wifi_started = false;
static bool wifi_connected = false;
static char config_ssid[32] = "ClaudeConfig";
static char config_password[16] = "Claude1234";
static char connected_ssid[33] = "";
static char station_ip[16] = "";
static bool sntp_started = false;
static bool sta_netif_created = false;
static bool ap_netif_created = false;

static void make_config_ssid(void)
{
    uint8_t mac[6] = {};
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    snprintf(config_ssid, sizeof(config_ssid), "ClaudeConfig-%02X%02X", mac[4], mac[5]);
    snprintf(config_password, sizeof(config_password), "Claude%02X%02X", mac[4], mac[5]);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        station_ip[0] = '\0';
        ESP_LOGW(TAG, "station disconnected, reconnecting");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_connected = true;
        ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(event_data);
        snprintf(station_ip, sizeof(station_ip), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "station got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        if (!sntp_started) {
            esp_sntp_config_t sntp_cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
            sntp_cfg.wait_for_sync = false;
            sntp_cfg.start = true;
            esp_err_t ret = esp_netif_sntp_init(&sntp_cfg);
            if (ret == ESP_OK) {
                sntp_started = true;
                ESP_LOGI(TAG, "SNTP started");
            } else if (ret != ESP_ERR_INVALID_STATE) {
                ESP_LOGW(TAG, "SNTP start failed: %s", esp_err_to_name(ret));
            }
        }
    }
}

static void ensure_wifi_started(void)
{
    if (wifi_started) {
        return;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, nullptr, nullptr));
    wifi_started = true;
}

static void ensure_sta_netif(void)
{
    if (!sta_netif_created) {
        esp_netif_create_default_wifi_sta();
        sta_netif_created = true;
    }
}

static void ensure_ap_netif(void)
{
    if (!ap_netif_created) {
        esp_netif_create_default_wifi_ap();
        ap_netif_created = true;
    }
}

static void start_station(const char *ssid, const char *password)
{
    ensure_sta_netif();
    ensure_wifi_started();

    wifi_config_t wifi_config = {};
    strlcpy(reinterpret_cast<char *>(wifi_config.sta.ssid), ssid, sizeof(wifi_config.sta.ssid));
    strlcpy(reinterpret_cast<char *>(wifi_config.sta.password), password, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    current_mode = CLAUDE_WIFI_MODE_STATION;
    strlcpy(connected_ssid, ssid, sizeof(connected_ssid));
    station_ip[0] = '\0';
    ESP_LOGI(TAG, "station connecting to %s", ssid);
}

void claude_wifi_service_enter_config_mode(void)
{
    make_config_ssid();
    ensure_sta_netif();
    ensure_ap_netif();
    ensure_wifi_started();

    wifi_config_t ap_config = {};
    strlcpy(reinterpret_cast<char *>(ap_config.ap.ssid), config_ssid, sizeof(ap_config.ap.ssid));
    ap_config.ap.ssid_len = strlen(config_ssid);
    ap_config.ap.channel = 1;
    ap_config.ap.max_connection = 4;
    strlcpy(reinterpret_cast<char *>(ap_config.ap.password), config_password, sizeof(ap_config.ap.password));
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    current_mode = CLAUDE_WIFI_MODE_CONFIG_AP;
    wifi_connected = false;
    ESP_LOGI(TAG, "config AP started: %s, password: %s, open http://192.168.4.1/", config_ssid, config_password);
}

esp_err_t claude_wifi_service_scan_networks(wifi_ap_record_t *records, uint16_t *count, uint16_t max_count)
{
    if (records == nullptr || count == nullptr || max_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (current_mode != CLAUDE_WIFI_MODE_CONFIG_AP) {
        return ESP_ERR_INVALID_STATE;
    }

    ensure_sta_netif();
    ensure_wifi_started();

    wifi_scan_config_t scan_config = {};
    scan_config.show_hidden = true;

    esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
    if (ret != ESP_OK) {
        return ret;
    }

    uint16_t available = max_count;
    ret = esp_wifi_scan_get_ap_records(&available, records);
    if (ret != ESP_OK) {
        esp_wifi_clear_ap_list();
        return ret;
    }

    *count = available;
    return ESP_OK;
}

void claude_wifi_service_start(void)
{
    char ssid[33] = {};
    char password[65] = {};
    claude_config_get_wifi_ssid(ssid, sizeof(ssid));
    claude_config_get_wifi_password(password, sizeof(password));

    if (ssid[0] == '\0') {
        claude_wifi_service_enter_config_mode();
    } else {
        start_station(ssid, password);
    }
}

claude_wifi_mode_t claude_wifi_service_get_mode(void)
{
    return current_mode;
}

bool claude_wifi_service_is_connected(void)
{
    return wifi_connected;
}

const char *claude_wifi_service_get_config_ssid(void)
{
    return config_ssid;
}

const char *claude_wifi_service_get_config_password(void)
{
    return config_password;
}

const char *claude_wifi_service_get_connected_ssid(void)
{
    return connected_ssid;
}

const char *claude_wifi_service_get_station_ip(void)
{
    return station_ip;
}

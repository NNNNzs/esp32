#include "claude_config_web.h"

#include "claude_config_service.h"
#include "esp_http_server.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "ClaudeConfigWeb";
static httpd_handle_t server = nullptr;

static void url_decode(char *value)
{
    char *read = value;
    char *write = value;

    while (*read != '\0') {
        if (*read == '+') {
            *write++ = ' ';
            read++;
        } else if (*read == '%' && read[1] != '\0' && read[2] != '\0') {
            char hex[3] = { read[1], read[2], '\0' };
            *write++ = (char)strtol(hex, nullptr, 16);
            read += 3;
        } else {
            *write++ = *read++;
        }
    }
    *write = '\0';
}

static bool get_form_value(const char *body, const char *key, char *out, size_t out_size)
{
    if (body == nullptr || key == nullptr || out == nullptr || out_size == 0) {
        return false;
    }

    size_t key_len = strlen(key);
    const char *pos = body;

    while ((pos = strstr(pos, key)) != nullptr) {
        if ((pos == body || pos[-1] == '&') && pos[key_len] == '=') {
            pos += key_len + 1;
            const char *end = strchr(pos, '&');
            size_t len = end ? (size_t)(end - pos) : strlen(pos);
            if (len >= out_size) {
                len = out_size - 1;
            }
            memcpy(out, pos, len);
            out[len] = '\0';
            url_decode(out);
            return true;
        }
        pos += key_len;
    }

    return false;
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    char broker[128] = {};
    char topic[128] = {};
    char ssid[33] = {};
    claude_config_get_mqtt_broker_uri(broker, sizeof(broker));
    claude_config_get_mqtt_topic(topic, sizeof(topic));
    claude_config_get_wifi_ssid(ssid, sizeof(ssid));

    char page[1800] = {};
    snprintf(page, sizeof(page),
        "<!doctype html><html><head>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Claude Config</title>"
        "<style>"
        "body{font-family:system-ui;margin:24px;background:#11151b;color:#eef2f7}"
        "label{display:block;margin:16px 0 6px;color:#aeb7c5}"
        "input{box-sizing:border-box;width:100%%;padding:12px;border-radius:8px;border:1px solid #3a4352;background:#171d25;color:#fff}"
        "button{margin-top:18px;width:100%%;padding:12px;border:0;border-radius:8px;background:#6bd3ff;color:#061018;font-weight:700}"
        ".hint{color:#aeb7c5;font-size:14px;line-height:1.45}"
        "</style></head><body>"
        "<h2>Claude Device Setup</h2>"
        "<p class='hint'>Set Wi-Fi and MQTT. If this page opened from the config hotspot, save and restart the device.</p>"
        "<form method='post' action='/mqtt'>"
        "<label>Wi-Fi SSID</label>"
        "<input name='ssid' value='%s' placeholder='Your Wi-Fi name'>"
        "<label>Wi-Fi Password</label>"
        "<input name='password' type='password' placeholder='Leave blank to keep current password'>"
        "<label>Broker URI</label>"
        "<input name='broker' value='%s' placeholder='mqtt://192.168.1.2:1883'>"
        "<label>Topic</label>"
        "<input name='topic' value='%s' placeholder='claudecode/device/default/status'>"
        "<button type='submit'>Save</button>"
        "</form></body></html>",
        ssid,
        broker,
        topic);

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, page, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t mqtt_post_handler(httpd_req_t *req)
{
    char body[512] = {};
    int total = 0;

    while (total < req->content_len && total < (int)sizeof(body) - 1) {
        int read = httpd_req_recv(req, body + total, sizeof(body) - 1 - total);
        if (read <= 0) {
            return ESP_FAIL;
        }
        total += read;
    }
    body[total] = '\0';

    char broker[128] = {};
    char topic[128] = {};
    char ssid[33] = {};
    char password[65] = {};
    bool have_broker = get_form_value(body, "broker", broker, sizeof(broker));
    bool have_topic = get_form_value(body, "topic", topic, sizeof(topic));
    bool have_ssid = get_form_value(body, "ssid", ssid, sizeof(ssid));
    bool have_password = get_form_value(body, "password", password, sizeof(password));

    if (have_ssid && ssid[0] != '\0') {
        if (!have_password || password[0] == '\0') {
            claude_config_get_wifi_password(password, sizeof(password));
        }
        claude_config_set_wifi_credentials(ssid, password);
    }
    if (have_broker) {
        claude_config_set_mqtt_broker_uri(broker);
    }
    if (have_topic) {
        claude_config_set_mqtt_topic(topic);
    }

    ESP_LOGI(TAG, "saved config ssid=%s broker=%s topic=%s", ssid, broker, topic);
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    return httpd_resp_send(req, nullptr, 0);
}

bool claude_config_web_start(void)
{
    if (server != nullptr) {
        return true;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "failed to start config web server");
        server = nullptr;
        return false;
    }

    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = nullptr,
    };
    httpd_register_uri_handler(server, &root_uri);

    httpd_uri_t mqtt_uri = {
        .uri = "/mqtt",
        .method = HTTP_POST,
        .handler = mqtt_post_handler,
        .user_ctx = nullptr,
    };
    httpd_register_uri_handler(server, &mqtt_uri);

    ESP_LOGI(TAG, "config web server started");
    return true;
}

bool claude_config_web_is_running(void)
{
    return server != nullptr;
}

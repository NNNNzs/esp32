#include "claude_config_web.h"

#include "claude_config_service.h"
#include "claude_wifi_service.h"
#include "esp_http_server.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "ClaudeConfigWeb";
static httpd_handle_t server = nullptr;

static bool send_chunkf(httpd_req_t *req, const char *fmt, ...)
{
    char buffer[256] = {};
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if (written < 0) {
        return false;
    }
    return httpd_resp_sendstr_chunk(req, buffer) == ESP_OK;
}

static bool send_chunk(httpd_req_t *req, const char *text)
{
    return httpd_resp_sendstr_chunk(req, text) == ESP_OK;
}

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
    char mqtt_username[64] = {};
    char ssid[33] = {};
    claude_config_get_mqtt_broker_uri(broker, sizeof(broker));
    claude_config_get_mqtt_topic(topic, sizeof(topic));
    claude_config_get_mqtt_username(mqtt_username, sizeof(mqtt_username));
    claude_config_get_wifi_ssid(ssid, sizeof(ssid));

    httpd_resp_set_type(req, "text/html");
    if (!send_chunk(req,
            "<!doctype html><html><head>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<title>Claude Config</title>"
            "<style>"
            "body{font-family:system-ui;margin:24px;background:#11151b;color:#eef2f7}"
            "label{display:block;margin:16px 0 6px;color:#aeb7c5}"
            "input{box-sizing:border-box;width:100%;padding:12px;border-radius:8px;border:1px solid #3a4352;background:#171d25;color:#fff}"
            "button{margin-top:18px;width:100%;padding:12px;border:0;border-radius:8px;background:#6bd3ff;color:#061018;font-weight:700}"
            "button.secondary{background:#263243;color:#eef2f7}"
            ".row{display:flex;gap:10px;align-items:center}"
            ".row>*{flex:1}"
            ".hint{color:#aeb7c5;font-size:14px;line-height:1.45}"
            "</style></head><body>"
            "<h2>Claude Device Setup</h2>"
            "<p class='hint'>Set Wi-Fi and MQTT. If this page opened from the config hotspot, save and restart the device.</p>"
            "<form method='post' action='/mqtt'>"
            "<label>Wi-Fi SSID</label>"
            "<div class='row'>"
            "<input name='ssid' id='ssid' list='wifi_list' value='")) {
        return ESP_FAIL;
    }
    if (!send_chunkf(req, "%s", ssid) ||
        !send_chunk(req,
            "' placeholder='Choose or type your Wi-Fi name'>"
            "<button class='secondary' type='button' id='scan-btn'>Scan</button>"
            "</div>"
            "<datalist id='wifi_list'></datalist>"
            "<label>Wi-Fi Password</label>"
            "<input name='password' type='password' placeholder='Leave blank to keep current password'>"
            "<label>Broker URI</label>"
            "<input name='broker' value='")) {
        return ESP_FAIL;
    }
    if (!send_chunkf(req, "%s", broker) ||
        !send_chunk(req,
            "' placeholder='mqtt://192.168.1.2:1883'>"
            "<label>Topic</label>"
            "<input name='topic' value='")) {
        return ESP_FAIL;
    }
    if (!send_chunkf(req, "%s", topic) ||
        !send_chunk(req,
            "' placeholder='claudecode/device/default/status'>"
            "<label>MQTT Username</label>"
            "<input name='mqtt_username' value='")) {
        return ESP_FAIL;
    }
    if (!send_chunkf(req, "%s", mqtt_username) ||
        !send_chunk(req,
            "' placeholder='Leave blank to keep current username'>"
            "<label>MQTT Password</label>"
            "<input name='mqtt_password' type='password' placeholder='Leave blank to keep current password'>"
            "<button type='submit'>Save</button>"
            "</form>"
            "<script>"
            "const scanBtn=document.getElementById('scan-btn');"
            "const wifiList=document.getElementById('wifi_list');"
            "const ssidInput=document.getElementById('ssid');"
            "scanBtn.addEventListener('click',async()=>{"
            "scanBtn.disabled=true;scanBtn.textContent='...';"
            "try{"
            "const resp=await fetch('/scan');"
            "if(!resp.ok)throw new Error('scan');"
            "const text=await resp.text();"
            "wifiList.innerHTML='';"
            "text.trim().split(/\\n+/).filter(Boolean).forEach(line=>{"
            "const parts=line.split('\\t');"
            "const ssid=parts[0]||'';"
            "const rssi=parts[1]||'';"
            "const opt=document.createElement('option');"
            "opt.value=ssid;"
            "opt.label=rssi?`${ssid} (${rssi} dBm)`:ssid;"
            "wifiList.appendChild(opt);"
            "});"
            "if(!ssidInput.value&&wifiList.options.length>0){ssidInput.value=wifiList.options[0].value;}"
            "}catch(e){console.log(e);}"
            "finally{scanBtn.disabled=false;scanBtn.textContent='Scan';}"
            "});"
            "</script></body></html>")) {
        return ESP_FAIL;
    }

    return httpd_resp_sendstr_chunk(req, NULL);
}

static esp_err_t scan_get_handler(httpd_req_t *req)
{
    wifi_ap_record_t records[20] = {};
    uint16_t count = 20;

    esp_err_t ret = claude_wifi_service_scan_networks(records, &count, 20);
    if (ret != ESP_OK) {
        httpd_resp_set_status(req, "503 Service Unavailable");
        httpd_resp_set_type(req, "text/plain");
        return httpd_resp_sendstr(req, "scan failed");
    }

    httpd_resp_set_type(req, "text/plain");
    for (uint16_t i = 0; i < count; ++i) {
        char line[160] = {};
        snprintf(line, sizeof(line), "%s\t%d\n", (const char *)records[i].ssid, records[i].rssi);
        if (httpd_resp_sendstr_chunk(req, line) != ESP_OK) {
            return ESP_FAIL;
        }
    }

    return httpd_resp_sendstr_chunk(req, NULL);
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
    char mqtt_username[64] = {};
    char mqtt_password[64] = {};
    char ssid[33] = {};
    char password[65] = {};
    bool have_broker = get_form_value(body, "broker", broker, sizeof(broker));
    bool have_topic = get_form_value(body, "topic", topic, sizeof(topic));
    bool have_mqtt_username = get_form_value(body, "mqtt_username", mqtt_username, sizeof(mqtt_username));
    bool have_mqtt_password = get_form_value(body, "mqtt_password", mqtt_password, sizeof(mqtt_password));
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
    if (have_mqtt_username || have_mqtt_password) {
        char current_username[64] = {};
        char current_password[64] = {};
        if (!have_mqtt_username || mqtt_username[0] == '\0') {
            claude_config_get_mqtt_username(current_username, sizeof(current_username));
            strlcpy(mqtt_username, current_username, sizeof(mqtt_username));
        }
        if (!have_mqtt_password || mqtt_password[0] == '\0') {
            claude_config_get_mqtt_password(current_password, sizeof(current_password));
            strlcpy(mqtt_password, current_password, sizeof(mqtt_password));
        }
        claude_config_set_mqtt_credentials(mqtt_username, mqtt_password);
    }

    ESP_LOGI(TAG, "saved config ssid=%s broker=%s topic=%s mqtt_user=%s", ssid, broker, topic, mqtt_username);
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
    config.stack_size = 8192;

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

    httpd_uri_t scan_uri = {
        .uri = "/scan",
        .method = HTTP_GET,
        .handler = scan_get_handler,
        .user_ctx = nullptr,
    };
    httpd_register_uri_handler(server, &scan_uri);

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

bool claude_config_web_stop(void)
{
    if (server == nullptr) {
        return true;
    }

    esp_err_t ret = httpd_stop(server);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to stop config web server: %s", esp_err_to_name(ret));
        return false;
    }

    server = nullptr;
    ESP_LOGI(TAG, "config web server stopped");
    return true;
}

bool claude_config_web_is_running(void)
{
    return server != nullptr;
}

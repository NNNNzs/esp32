#include "WifiSettings.hpp"
#include "claude_config_service.h"
#include "claude_config_web.h"
#include "claude_mqtt_service.h"
#include "claude_wifi_service.h"

#include <stdio.h>

LV_IMG_DECLARE(img_app_drawpanel);

WifiSettings::WifiSettings() : ESP_Brookesia_PhoneApp("WiFi", &img_app_drawpanel, true)
{
}

WifiSettings::~WifiSettings()
{
}

bool WifiSettings::init(void)
{
    return true;
}

bool WifiSettings::run(void)
{
    lv_area_t area = getVisualArea();
    int width = area.x2 - area.x1;
    int height = area.y2 - area.y1;

    root_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root_, width, height);
    lv_obj_align(root_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(root_, lv_color_make(14, 18, 24), 0);
    lv_obj_set_style_border_width(root_, 0, 0);
    lv_obj_set_style_radius(root_, 0, 0);
    lv_obj_set_style_pad_all(root_, 12, 0);
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(root_);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_make(236, 238, 244), 0);
    lv_label_set_text(title, "WiFi Setup");

    wifi_status_label_ = lv_label_create(root_);
    lv_obj_set_width(wifi_status_label_, width - 24);
    lv_obj_set_style_text_font(wifi_status_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(wifi_status_label_, lv_color_make(170, 176, 190), 0);
    lv_obj_set_style_text_align(wifi_status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(wifi_status_label_, LV_LABEL_LONG_WRAP);

    wifi_ip_label_ = lv_label_create(root_);
    lv_obj_set_width(wifi_ip_label_, width - 24);
    lv_obj_set_style_text_font(wifi_ip_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(wifi_ip_label_, lv_color_make(170, 176, 190), 0);
    lv_obj_set_style_text_align(wifi_ip_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(wifi_ip_label_, LV_LABEL_LONG_WRAP);

    config_web_status_label_ = lv_label_create(root_);
    lv_obj_set_width(config_web_status_label_, width - 24);
    lv_obj_set_style_text_font(config_web_status_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(config_web_status_label_, lv_color_make(170, 176, 190), 0);
    lv_obj_set_style_text_align(config_web_status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(config_web_status_label_, LV_LABEL_LONG_WRAP);

    mqtt_status_label_ = lv_label_create(root_);
    lv_obj_set_width(mqtt_status_label_, width - 24);
    lv_obj_set_style_text_font(mqtt_status_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(mqtt_status_label_, lv_color_make(170, 176, 190), 0);
    lv_obj_set_style_text_align(mqtt_status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(mqtt_status_label_, LV_LABEL_LONG_WRAP);

    config_web_btn_ = lv_btn_create(root_);
    lv_obj_set_width(config_web_btn_, width - 24);
    lv_obj_set_height(config_web_btn_, 44);
    lv_obj_add_event_cb(config_web_btn_, config_web_btn_cb, LV_EVENT_CLICKED, this);
    config_web_btn_label_ = lv_label_create(config_web_btn_);
    lv_obj_center(config_web_btn_label_);

    mqtt_btn_ = lv_btn_create(root_);
    lv_obj_set_width(mqtt_btn_, width - 24);
    lv_obj_set_height(mqtt_btn_, 44);
    lv_obj_add_event_cb(mqtt_btn_, mqtt_btn_cb, LV_EVENT_CLICKED, this);
    mqtt_btn_label_ = lv_label_create(mqtt_btn_);
    lv_obj_center(mqtt_btn_label_);

    refresh_ui();
    refresh_timer_ = lv_timer_create(refresh_timer_cb, 1000, this);

    return true;
}

bool WifiSettings::back(void)
{
    notifyCoreClosed();
    return true;
}

bool WifiSettings::close(void)
{
    if (refresh_timer_ != nullptr) {
        lv_timer_del(refresh_timer_);
        refresh_timer_ = nullptr;
    }
    root_ = nullptr;
    wifi_status_label_ = nullptr;
    wifi_ip_label_ = nullptr;
    config_web_status_label_ = nullptr;
    mqtt_status_label_ = nullptr;
    config_web_btn_ = nullptr;
    config_web_btn_label_ = nullptr;
    mqtt_btn_ = nullptr;
    mqtt_btn_label_ = nullptr;
    return true;
}

void WifiSettings::refresh_timer_cb(lv_timer_t *timer)
{
    WifiSettings *app = static_cast<WifiSettings *>(timer->user_data);
    if (app != nullptr) {
        app->refresh_ui();
    }
}

void WifiSettings::config_web_btn_cb(lv_event_t *event)
{
    WifiSettings *app = static_cast<WifiSettings *>(lv_event_get_user_data(event));
    if (app == nullptr) {
        return;
    }

    if (claude_config_web_is_running()) {
        claude_config_web_stop();
    } else {
        claude_config_web_start();
    }
    app->refresh_ui();
}

void WifiSettings::mqtt_btn_cb(lv_event_t *event)
{
    WifiSettings *app = static_cast<WifiSettings *>(lv_event_get_user_data(event));
    if (app == nullptr) {
        return;
    }

    if (claude_mqtt_service_is_running()) {
        claude_mqtt_service_stop();
    } else {
        claude_mqtt_service_start();
    }
    app->refresh_ui();
}

void WifiSettings::refresh_ui()
{
    if (wifi_status_label_ == nullptr || wifi_ip_label_ == nullptr || config_web_status_label_ == nullptr ||
            mqtt_status_label_ == nullptr || config_web_btn_ == nullptr || mqtt_btn_ == nullptr) {
        return;
    }

    char text[260] = {};
    if (claude_wifi_service_get_mode() == CLAUDE_WIFI_MODE_CONFIG_AP) {
        snprintf(text, sizeof(text), "Wi-Fi: setup hotspot %s", claude_wifi_service_get_config_ssid());
        lv_label_set_text(wifi_status_label_, text);
        snprintf(text, sizeof(text), "Hotspot password: %s", claude_wifi_service_get_config_password());
        lv_label_set_text(wifi_ip_label_, text);
    } else if (claude_wifi_service_is_connected()) {
        const char *ssid = claude_wifi_service_get_connected_ssid();
        const char *ip = claude_wifi_service_get_station_ip();
        snprintf(text, sizeof(text), "Wi-Fi: connected to %s", (ssid != nullptr && ssid[0] != '\0') ? ssid : "(unknown)");
        lv_label_set_text(wifi_status_label_, text);
        snprintf(text, sizeof(text), "Device IP: %s", (ip != nullptr && ip[0] != '\0') ? ip : "(waiting for IP)");
        lv_label_set_text(wifi_ip_label_, text);
    } else {
        lv_label_set_text(wifi_status_label_, "Wi-Fi: disconnected");
        lv_label_set_text(wifi_ip_label_, "Device IP: (none)");
    }

    if (claude_config_web_is_running()) {
        lv_label_set_text(config_web_status_label_, "Config web: running on port 80");
        lv_label_set_text(config_web_btn_label_, "Stop Web");
    } else {
        lv_label_set_text(config_web_status_label_, "Config web: stopped");
        lv_label_set_text(config_web_btn_label_, "Start Web");
    }

    if (claude_mqtt_service_is_running()) {
        char broker[128] = {};
        claude_config_get_mqtt_broker_uri(broker, sizeof(broker));
        if (claude_mqtt_service_is_connected()) {
            snprintf(text, sizeof(text), "MQTT: running and connected to %s", broker);
        } else {
            snprintf(text, sizeof(text), "MQTT: running, waiting for %s", broker);
        }
        lv_label_set_text(mqtt_status_label_, text);
        lv_label_set_text(mqtt_btn_label_, "Stop MQTT");
    } else {
        lv_label_set_text(mqtt_status_label_, "MQTT: stopped");
        lv_label_set_text(mqtt_btn_label_, "Start MQTT");
    }
}

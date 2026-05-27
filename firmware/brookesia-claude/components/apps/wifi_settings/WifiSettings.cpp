#include "WifiSettings.hpp"
#include "claude_config_web.h"
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

    lv_obj_t *hint = lv_label_create(root_);
    lv_obj_set_width(hint, width - 24);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(hint, lv_color_make(170, 176, 190), 0);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    if (claude_wifi_service_get_mode() == CLAUDE_WIFI_MODE_CONFIG_AP) {
        char text[160] = {};
        snprintf(text, sizeof(text), "Connect phone to %s, then open http://192.168.4.1/", claude_wifi_service_get_config_ssid());
        lv_label_set_text(hint, text);
    } else if (claude_wifi_service_is_connected()) {
        lv_label_set_text(hint, "Wi-Fi connected. Open this device IP in your phone browser to edit MQTT.");
    } else {
        lv_label_set_text(hint, claude_config_web_is_running()
            ? "Config page is running. Waiting for Wi-Fi."
            : "Config page is not running.");
    }

    return true;
}

bool WifiSettings::back(void)
{
    notifyCoreClosed();
    return true;
}

bool WifiSettings::close(void)
{
    root_ = nullptr;
    return true;
}

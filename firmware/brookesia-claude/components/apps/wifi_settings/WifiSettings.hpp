#pragma once

#include "lvgl.h"
#include "esp_brookesia.hpp"

class WifiSettings : public ESP_Brookesia_PhoneApp
{
public:
    WifiSettings();
    ~WifiSettings();

    bool run(void);
    bool back(void);
    bool close(void);
    bool init(void) override;

private:
    static void refresh_timer_cb(lv_timer_t *timer);
    static void config_web_btn_cb(lv_event_t *event);
    static void mqtt_btn_cb(lv_event_t *event);
    void refresh_ui();

    lv_obj_t *root_ = nullptr;
    lv_obj_t *wifi_status_label_ = nullptr;
    lv_obj_t *wifi_ip_label_ = nullptr;
    lv_obj_t *config_web_status_label_ = nullptr;
    lv_obj_t *mqtt_status_label_ = nullptr;
    lv_obj_t *config_web_btn_ = nullptr;
    lv_obj_t *config_web_btn_label_ = nullptr;
    lv_obj_t *mqtt_btn_ = nullptr;
    lv_obj_t *mqtt_btn_label_ = nullptr;
    lv_timer_t *refresh_timer_ = nullptr;
};

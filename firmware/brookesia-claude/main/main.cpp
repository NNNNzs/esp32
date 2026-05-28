/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include "esp_check.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "esp_brookesia.hpp"
#include "bsp_board_extra.h"
#include "bsp/esp32_s3_touch_lcd_1_83.h"
#include "claude_config_web.h"
#include "claude_mqtt_service.h"
#include "claude_wifi_service.h"

#include "app_examples/phone/squareline/src/phone_app_squareline.hpp"
#include "apps.h"


static const char *TAG = "main";

static void my_rounder_cb(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    area->x1 = (area->x1 >> 1) << 1;
    area->y1 = (area->y1 >> 1) << 1;
    area->x2 = ((area->x2 >> 1) << 1) + 1;
    area->y2 = ((area->y2 >> 1) << 1) + 1;
}

static void refresh_status_bar_cb(struct _lv_timer_t *t)
{
    ESP_Brookesia_Phone *phone = static_cast<ESP_Brookesia_Phone *>(t->user_data);
    if (phone == nullptr || phone->getHome().getStatusBar() == nullptr) {
        return;
    }

    ESP_Brookesia_StatusBar *status_bar = phone->getHome().getStatusBar();
    if (claude_wifi_service_get_mode() == CLAUDE_WIFI_MODE_CONFIG_AP) {
        status_bar->setWifiIconState(ESP_Brookesia_StatusBar::WifiState::SIGNAL_3);
    } else if (claude_wifi_service_is_connected()) {
        status_bar->setWifiIconState(ESP_Brookesia_StatusBar::WifiState::SIGNAL_3);
    } else {
        status_bar->setWifiIconState(ESP_Brookesia_StatusBar::WifiState::DISCONNECTED);
    }

    time_t now = 0;
    struct tm timeinfo = {};
    time(&now);
    if (now >= 1704067200) {    // 2024-01-01 UTC
        localtime_r(&now, &timeinfo);
        bool is_time_pm = (timeinfo.tm_hour >= 12);
        status_bar->setClock(timeinfo.tm_hour, timeinfo.tm_min, is_time_pm);
    }
}

static void log_directory_preview(const char *path, size_t max_entries)
{
    DIR *dir = opendir(path);
    if (!dir) {
        ESP_LOGW(TAG, "Cannot open directory: %s", path);
        return;
    }

    ESP_LOGI(TAG, "Directory preview for %s:", path);
    struct dirent *entry = NULL;
    size_t count = 0;
    while ((entry = readdir(dir)) != NULL && count < max_entries) {
        ESP_LOGI(TAG, "  %s", entry->d_name);
        count++;
    }

    if (count == 0) {
        ESP_LOGI(TAG, "  (empty)");
    }

    closedir(dir);
}

static void probe_sdcard(void)
{
    ESP_LOGI(TAG, "Probing SD card mount at %s", BSP_SD_MOUNT_POINT);
    esp_err_t ret = bsp_sdcard_mount();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SD card mounted successfully at %s", BSP_SD_MOUNT_POINT);
        if (bsp_sdcard != NULL) {
            ESP_LOGI(TAG, "SD card CID name: %s", bsp_sdcard->cid.name);
        }
        log_directory_preview(BSP_SD_MOUNT_POINT, 20);
        return;
    }

    ESP_LOGW(TAG, "SD card mount failed: %s", esp_err_to_name(ret));
    ESP_LOGW(TAG, "If you inserted a card, this usually means the card was not detected, the slot is not wired as expected, or the card is not FAT-formatted.");
}

extern "C" void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    err = esp_event_loop_create_default();
    if (err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }

    ESP_ERROR_CHECK(bsp_spiffs_mount());
    ESP_LOGI(TAG, "SPIFFS mount successfully");
    setenv("TZ", "CST-8", 1);
    tzset();

    probe_sdcard();

    ESP_ERROR_CHECK(bsp_extra_codec_init());
    claude_wifi_service_start();
    claude_config_web_start();
    claude_mqtt_service_start();

    lv_display_t *disp = bsp_display_start();

    if (disp && disp->driver) {
        disp->driver->rounder_cb = my_rounder_cb;
    }

    bsp_display_backlight_on();
    bsp_display_lock(0);
    ESP_Brookesia_Phone *phone = new ESP_Brookesia_Phone(disp);
    assert(phone != nullptr && "Failed to create phone");

    ESP_Brookesia_PhoneStylesheet_t *stylesheet = nullptr;
    stylesheet = new ESP_Brookesia_PhoneStylesheet_t ESP_BROOKESIA_PHONE_DEFAULT_DARK_STYLESHEET();
    ESP_BROOKESIA_CHECK_NULL_EXIT(stylesheet, "Create stylesheet failed");

    if (stylesheet != nullptr)
    {
        ESP_LOGI(TAG, "Using stylesheet (%s)", stylesheet->core.name);
        ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->addStylesheet(stylesheet), "Add stylesheet failed");
        ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->activateStylesheet(stylesheet), "Activate stylesheet failed");
        delete stylesheet;
    }

    ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->setTouchDevice(bsp_display_get_input_dev()), "Set touch device failed");
    phone->registerLvLockCallback((ESP_Brookesia_LvLockCallback_t)(bsp_display_lock), 0);
    phone->registerLvUnlockCallback((ESP_Brookesia_LvUnlockCallback_t)(bsp_display_unlock));
    ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->begin(), "Begin failed");

    MusicPlayer *music_player = new MusicPlayer();
    assert(music_player != nullptr && "Failed to create music_player");
    assert((phone->installApp(music_player) >= 0) && "Failed to begin music_player");

    Drawpanel *drawpanel = new Drawpanel();
    assert(drawpanel != nullptr && "Failed to create drawpanel");
    assert((phone->installApp(drawpanel) >= 0) && "Failed to begin drawpanel");

    ClaudeStatus *claude_status = new ClaudeStatus();
    assert(claude_status != nullptr && "Failed to create claude_status");
    assert((phone->installApp(claude_status) >= 0) && "Failed to begin claude_status");

    WifiSettings *wifi_settings = new WifiSettings();
    assert(wifi_settings != nullptr && "Failed to create wifi_settings");
    assert((phone->installApp(wifi_settings) >= 0) && "Failed to begin wifi_settings");

    PhoneAppSquareline *app_squareline = new PhoneAppSquareline();
    ESP_BROOKESIA_CHECK_NULL_EXIT(app_squareline, "Create app squareline failed");
    ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(app_squareline) >= 0), "Install app squareline failed");

    /* Refresh status bar Wi-Fi state and clock */
    ESP_BROOKESIA_CHECK_NULL_EXIT(lv_timer_create(refresh_status_bar_cb, 1000, phone), "Create status bar update timer failed");

    bsp_display_unlock();
}

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
    lv_obj_t *root_ = nullptr;
};

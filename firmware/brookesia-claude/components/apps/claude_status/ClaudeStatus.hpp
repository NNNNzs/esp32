#pragma once

#include "lvgl.h"
#include "esp_brookesia.hpp"

enum class ClaudeStatusState {
    Idle,
    Thinking,
    Working,
    Approval,
    Done,
    Error,
};

class ClaudeStatus : public ESP_Brookesia_PhoneApp
{
public:
    ClaudeStatus();
    ~ClaudeStatus();

    bool run(void);
    bool back(void);
    bool close(void);
    bool init(void) override;

    void setState(ClaudeStatusState state);

private:
    static void animation_timer_cb(lv_timer_t *timer);
    void refresh();

    lv_obj_t *root_ = nullptr;
    lv_obj_t *robot_label_ = nullptr;
    lv_obj_t *state_label_ = nullptr;
    lv_obj_t *hint_label_ = nullptr;
    lv_timer_t *animation_timer_ = nullptr;
    ClaudeStatusState state_ = ClaudeStatusState::Idle;
    uint8_t frame_ = 0;
};

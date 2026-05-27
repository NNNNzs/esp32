#include "ClaudeStatus.hpp"
#include "claude_status_service.h"

LV_IMG_DECLARE(img_app_drawpanel);

static const char *robot_idle =
    " .----.\n"
    "| o  o |\n"
    "|  --  |\n"
    "'-.__.-'";

static const char *robot_thinking_frames[] = {
    " .----.\n"
    "| o  o |\n"
    "|  ..  |\n"
    "'-.__.-'",
    " .----.\n"
    "| o  O |\n"
    "|  ..  |\n"
    "'-.__.-'",
    " .----.\n"
    "| O  o |\n"
    "|  ..  |\n"
    "'-.__.-'",
    " .----.\n"
    "| o  o |\n"
    "| ...  |\n"
    "'-.__.-'",
};

ClaudeStatus::ClaudeStatus() : ESP_Brookesia_PhoneApp("Claude", &img_app_drawpanel, true)
{
}

ClaudeStatus::~ClaudeStatus()
{
}

bool ClaudeStatus::init(void)
{
    return true;
}

bool ClaudeStatus::run(void)
{
    lv_area_t area = getVisualArea();
    int width = area.x2 - area.x1;
    int height = area.y2 - area.y1;

    root_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root_, width, height);
    lv_obj_align(root_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(root_, lv_color_make(12, 14, 18), 0);
    lv_obj_set_style_border_width(root_, 0, 0);
    lv_obj_set_style_radius(root_, 0, 0);
    lv_obj_set_style_pad_all(root_, 8, 0);
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);

    robot_label_ = lv_label_create(root_);
    lv_obj_set_style_text_font(robot_label_, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(robot_label_, lv_color_make(236, 238, 244), 0);
    lv_obj_set_style_text_align(robot_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(robot_label_, robot_idle);

    state_label_ = lv_label_create(root_);
    lv_obj_set_style_text_font(state_label_, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(state_label_, lv_color_make(130, 210, 255), 0);
    lv_obj_set_style_text_align(state_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(state_label_, "IDLE");

    hint_label_ = lv_label_create(root_);
    lv_obj_set_style_text_font(hint_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(hint_label_, lv_color_make(170, 176, 190), 0);
    lv_obj_set_style_text_align(hint_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(hint_label_, "waiting for MQTT");

    animation_timer_ = lv_timer_create(animation_timer_cb, 450, this);
    refresh();

    return true;
}

bool ClaudeStatus::back(void)
{
    notifyCoreClosed();
    return true;
}

bool ClaudeStatus::close(void)
{
    if (animation_timer_ != nullptr) {
        lv_timer_del(animation_timer_);
        animation_timer_ = nullptr;
    }
    root_ = nullptr;
    robot_label_ = nullptr;
    state_label_ = nullptr;
    hint_label_ = nullptr;
    return true;
}

void ClaudeStatus::setState(ClaudeStatusState state)
{
    state_ = state;
    frame_ = 0;
    refresh();
}

void ClaudeStatus::animation_timer_cb(lv_timer_t *timer)
{
    ClaudeStatus *app = static_cast<ClaudeStatus *>(timer->user_data);
    if (app == nullptr) {
        return;
    }

    app->frame_++;
    switch (claude_status_service_get_state()) {
    case CLAUDE_STATUS_THINKING:
        app->state_ = ClaudeStatusState::Thinking;
        break;
    case CLAUDE_STATUS_WORKING:
        app->state_ = ClaudeStatusState::Working;
        break;
    case CLAUDE_STATUS_APPROVAL:
        app->state_ = ClaudeStatusState::Approval;
        break;
    case CLAUDE_STATUS_DONE:
        app->state_ = ClaudeStatusState::Done;
        break;
    case CLAUDE_STATUS_ERROR:
        app->state_ = ClaudeStatusState::Error;
        break;
    case CLAUDE_STATUS_IDLE:
    default:
        app->state_ = ClaudeStatusState::Idle;
        break;
    }
    app->refresh();
}

void ClaudeStatus::refresh()
{
    if (robot_label_ == nullptr || state_label_ == nullptr || hint_label_ == nullptr) {
        return;
    }

    switch (state_) {
    case ClaudeStatusState::Thinking:
        lv_label_set_text(robot_label_, robot_thinking_frames[frame_ % 4]);
        lv_label_set_text(state_label_, "THINKING");
        lv_label_set_text(hint_label_, "Claude is thinking");
        lv_obj_set_style_text_color(state_label_, lv_color_make(130, 210, 255), 0);
        break;
    case ClaudeStatusState::Working:
        lv_label_set_text(robot_label_, frame_ % 2 == 0 ? "[= o_o =]" : "[= -_- =]");
        lv_label_set_text(state_label_, "WORKING");
        lv_label_set_text(hint_label_, "tool is running");
        lv_obj_set_style_text_color(state_label_, lv_color_make(118, 230, 180), 0);
        break;
    case ClaudeStatusState::Approval:
        lv_label_set_text(robot_label_, " .----.\n| !  ! |\n| ask? |\n'-.__.-'");
        lv_label_set_text(state_label_, "APPROVAL");
        lv_label_set_text(hint_label_, "waiting for you");
        lv_obj_set_style_text_color(state_label_, lv_color_make(255, 206, 110), 0);
        break;
    case ClaudeStatusState::Done:
        lv_label_set_text(robot_label_, " .----.\n| ^  ^ |\n| done |\n'-.__.-'");
        lv_label_set_text(state_label_, "DONE");
        lv_label_set_text(hint_label_, "ready");
        lv_obj_set_style_text_color(state_label_, lv_color_make(132, 226, 146), 0);
        break;
    case ClaudeStatusState::Error:
        lv_label_set_text(robot_label_, " .----.\n| x  x |\n| err  |\n'-.__.-'");
        lv_label_set_text(state_label_, "ERROR");
        lv_label_set_text(hint_label_, "check MQTT/logs");
        lv_obj_set_style_text_color(state_label_, lv_color_make(255, 110, 120), 0);
        break;
    case ClaudeStatusState::Idle:
    default:
        lv_label_set_text(robot_label_, robot_idle);
        lv_label_set_text(state_label_, "IDLE");
        lv_label_set_text(hint_label_, "waiting for MQTT");
        lv_obj_set_style_text_color(state_label_, lv_color_make(130, 210, 255), 0);
        break;
    }
}

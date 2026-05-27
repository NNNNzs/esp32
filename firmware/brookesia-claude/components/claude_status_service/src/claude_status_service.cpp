#include "claude_status_service.h"

#include <string.h>

static claude_status_state_t current_state = CLAUDE_STATUS_IDLE;

void claude_status_service_set_state(claude_status_state_t state)
{
    current_state = state;
}

void claude_status_service_set_state_from_text(const char *state)
{
    if (state == nullptr) {
        current_state = CLAUDE_STATUS_IDLE;
        return;
    }

    if (strcmp(state, "thinking") == 0) {
        current_state = CLAUDE_STATUS_THINKING;
    } else if (strcmp(state, "working") == 0) {
        current_state = CLAUDE_STATUS_WORKING;
    } else if (strcmp(state, "approval") == 0) {
        current_state = CLAUDE_STATUS_APPROVAL;
    } else if (strcmp(state, "done") == 0) {
        current_state = CLAUDE_STATUS_DONE;
    } else if (strcmp(state, "error") == 0) {
        current_state = CLAUDE_STATUS_ERROR;
    } else {
        current_state = CLAUDE_STATUS_IDLE;
    }
}

claude_status_state_t claude_status_service_get_state(void)
{
    return current_state;
}

const char *claude_status_service_get_state_text(void)
{
    switch (current_state) {
    case CLAUDE_STATUS_THINKING:
        return "thinking";
    case CLAUDE_STATUS_WORKING:
        return "working";
    case CLAUDE_STATUS_APPROVAL:
        return "approval";
    case CLAUDE_STATUS_DONE:
        return "done";
    case CLAUDE_STATUS_ERROR:
        return "error";
    case CLAUDE_STATUS_IDLE:
    default:
        return "idle";
    }
}

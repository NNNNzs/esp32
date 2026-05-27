#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CLAUDE_STATUS_IDLE = 0,
    CLAUDE_STATUS_THINKING,
    CLAUDE_STATUS_WORKING,
    CLAUDE_STATUS_APPROVAL,
    CLAUDE_STATUS_DONE,
    CLAUDE_STATUS_ERROR,
} claude_status_state_t;

void claude_status_service_set_state(claude_status_state_t state);
void claude_status_service_set_state_from_text(const char *state);
claude_status_state_t claude_status_service_get_state(void);
const char *claude_status_service_get_state_text(void);

#ifdef __cplusplus
}
#endif

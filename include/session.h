#ifndef SESSION_H
#define SESSION_H

typedef struct session_manager session_manager_t;

typedef enum {
    SESSION_STATE_DISCONNECTED = 0,
    SESSION_STATE_AUTHENTICATING,
    SESSION_STATE_MAIN_MENU,
    SESSION_STATE_BOARD_VIEW
} session_state_t;

session_manager_t *session_manager_create(void);
void session_manager_destroy(session_manager_t *manager);
void session_manager_simulate(session_manager_t *manager);

#endif // SESSION_H

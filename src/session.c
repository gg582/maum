#include "session.h"

#include "log.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>

#define COMPONENT "session"

struct session_manager {
    unsigned int simulated_sessions;
};

session_manager_t *session_manager_create(void)
{
    session_manager_t *manager = calloc(1, sizeof(*manager));
    if (manager == NULL) {
        return NULL;
    }
    return manager;
}

void session_manager_destroy(session_manager_t *manager)
{
    if (manager == NULL) {
        return;
    }
    free(manager);
}

void session_manager_simulate(session_manager_t *manager)
{
    if (manager == NULL) {
        return;
    }

    LOG_INFO(COMPONENT, "%s", "Simulating interactive session");
    manager->simulated_sessions++;

    menu_context_t menu;
    menu_init(&menu);
    menu_render_banner(&menu);
    menu_render_main(&menu);
}

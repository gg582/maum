#ifndef SESSION_H
#define SESSION_H

#include "board.h"
#include "config.h"

#include <stdio.h>

typedef struct session_manager session_manager_t;

typedef enum {
    SESSION_TRANSPORT_TELNET = 0,
    SESSION_TRANSPORT_SSH,
    SESSION_TRANSPORT_STDIO
} session_transport_t;

session_manager_t *session_manager_create(const maum_config_t *config);
void session_manager_destroy(session_manager_t *manager);

void session_manager_run(session_manager_t *manager,
                         session_transport_t transport,
                         FILE *input,
                         FILE *output,
                         const char *peer_identity);

#endif // SESSION_H

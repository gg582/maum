#ifndef SERVER_H
#define SERVER_H

#include "config.h"

typedef struct server_context server_context_t;

server_context_t *server_create(const maum_config_t *config);
void server_destroy(server_context_t *ctx);
int server_run(server_context_t *ctx);

#endif // SERVER_H

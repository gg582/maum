#include "server.h"

#include "log.h"
#include "maum.h"
#include "session.h"

#include <stdlib.h>
#include <string.h>

#define COMPONENT "server"

struct server_context {
    maum_config_t config;
};

server_context_t *server_create(const maum_config_t *config)
{
    if (config == NULL) {
        return NULL;
    }

    server_context_t *ctx = calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
        return NULL;
    }

    ctx->config = *config;
    return ctx;
}

void server_destroy(server_context_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    free(ctx);
}

int server_run(server_context_t *ctx)
{
    if (ctx == NULL) {
        return -1;
    }

    LOG_INFO(COMPONENT, "Starting %s v%s", MAUM_APP_NAME, MAUM_APP_VERSION);
    LOG_INFO(COMPONENT, "SSH  listening on %s:%u", ctx->config.ssh_host, ctx->config.ssh_port);
    LOG_INFO(COMPONENT, "TELNET listening on %s:%u", ctx->config.telnet_host, ctx->config.telnet_port);

    LOG_INFO(COMPONENT, "%s", "Initializing session manager");
    session_manager_t *sessions = session_manager_create();
    if (sessions == NULL) {
        LOG_ERROR(COMPONENT, "%s", "Unable to create session manager");
        return -1;
    }

    LOG_INFO(COMPONENT, "%s", "Session manager ready - awaiting connections (mock mode)");

    session_manager_simulate(sessions);

    session_manager_destroy(sessions);
    LOG_INFO(COMPONENT, "%s", "Server shutdown complete");
    return 0;
}

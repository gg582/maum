#include "config.h"
#include "log.h"
#include "maum.h"
#include "server.h"
#include "session.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *program)
{
    fprintf(stderr, "Usage: %s [--config path] [--log-level level] [--stdio]\n", program);
}

static log_level_t parse_log_level(const char *value)
{
    if (value == NULL) {
        return LOG_LEVEL_INFO;
    }
    if (strcmp(value, "debug") == 0) {
        return LOG_LEVEL_DEBUG;
    }
    if (strcmp(value, "info") == 0) {
        return LOG_LEVEL_INFO;
    }
    if (strcmp(value, "warn") == 0) {
        return LOG_LEVEL_WARN;
    }
    if (strcmp(value, "error") == 0) {
        return LOG_LEVEL_ERROR;
    }
    return LOG_LEVEL_INFO;
}

int main(int argc, char *argv[])
{
    const char *config_path = "maum.conf";
    log_level_t level = LOG_LEVEL_INFO;
    bool stdio_mode = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            config_path = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--log-level") == 0 && i + 1 < argc) {
            level = parse_log_level(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "--stdio") == 0) {
            stdio_mode = true;
            continue;
        }
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    log_set_level(level);

    maum_config_t config;
    config_init(&config);
    config_load(&config, config_path);

    if (stdio_mode) {
        session_manager_t *sessions = session_manager_create(&config);
        if (sessions == NULL) {
            LOG_ERROR("main", "%s", "세션 매니저를 초기화할 수 없습니다");
            return EXIT_FAILURE;
        }
        session_manager_run(sessions, SESSION_TRANSPORT_STDIO, stdin, stdout, "local");
        session_manager_destroy(sessions);
        return EXIT_SUCCESS;
    }

    server_context_t *server = server_create(&config);
    if (server == NULL) {
        LOG_ERROR("main", "%s", "Failed to initialize server context");
        return EXIT_FAILURE;
    }

    int result = server_run(server);
    server_destroy(server);

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

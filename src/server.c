#include "server.h"

#include "log.h"
#include "maum.h"
#include "session.h"
#include "telnet.h"

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define COMPONENT "server"

#define PEER_HOST_MAX 128
#define PEER_SERVICE_MAX 16

struct server_context {
    maum_config_t config;
    session_manager_t *sessions;
    int running;
    int telnet_listen_fd;
};

struct client_args {
    session_manager_t *sessions;
    int fd;
    session_transport_t transport;
    char peer[PEER_HOST_MAX + PEER_SERVICE_MAX + 2];
};

static server_context_t *g_server = NULL;

static void handle_signal(int signum)
{
    (void)signum;
    if (g_server == NULL) {
        return;
    }
    g_server->running = 0;
    if (g_server->telnet_listen_fd >= 0) {
        close(g_server->telnet_listen_fd);
        g_server->telnet_listen_fd = -1;
    }
}

static void *client_thread(void *arg)
{
    struct client_args *client = arg;
    if (client == NULL) {
        return NULL;
    }

    FILE *input = fdopen(client->fd, "r");
    if (input == NULL) {
        LOG_WARN(COMPONENT, "%s", "fdopen failed for client input");
        close(client->fd);
        free(client);
        return NULL;
    }

    int dup_fd = dup(client->fd);
    if (dup_fd < 0) {
        LOG_WARN(COMPONENT, "%s", "dup failed for client output");
        fclose(input);
        free(client);
        return NULL;
    }

    FILE *output = fdopen(dup_fd, "w");
    if (output == NULL) {
        LOG_WARN(COMPONENT, "%s", "fdopen failed for client output");
        fclose(input);
        close(dup_fd);
        free(client);
        return NULL;
    }

    setvbuf(input, NULL, _IONBF, 0);
    setvbuf(output, NULL, _IONBF, 0);

    if (client->transport == SESSION_TRANSPORT_TELNET) {
        telnet_send_initial_negotiation(output);
    }

    session_manager_run(client->sessions, client->transport, input, output, client->peer);

    fclose(output);
    fclose(input);
    free(client);
    return NULL;
}

static int open_listen_socket(const char *host, unsigned short port)
{
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *result = NULL;
    int rc = getaddrinfo((host != NULL && host[0] != '\0') ? host : NULL, port_str, &hints, &result);
    if (rc != 0) {
        LOG_ERROR(COMPONENT, "getaddrinfo failed: %s", gai_strerror(rc));
        return -1;
    }

    int listen_fd = -1;
    for (struct addrinfo *res = result; res != NULL; res = res->ai_next) {
        listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listen_fd < 0) {
            continue;
        }

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(listen_fd, res->ai_addr, res->ai_addrlen) == 0) {
            if (listen(listen_fd, 16) == 0) {
                break;
            }
        }

        close(listen_fd);
        listen_fd = -1;
    }

    freeaddrinfo(result);
    return listen_fd;
}

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
    ctx->telnet_listen_fd = -1;
    ctx->running = 1;

    ctx->sessions = session_manager_create(config);
    if (ctx->sessions == NULL) {
        free(ctx);
        return NULL;
    }

    return ctx;
}

void server_destroy(server_context_t *ctx)
{
    if (ctx == NULL) {
        return;
    }

    if (ctx->telnet_listen_fd >= 0) {
        close(ctx->telnet_listen_fd);
    }

    session_manager_destroy(ctx->sessions);
    free(ctx);
}

int server_run(server_context_t *ctx)
{
    if (ctx == NULL) {
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    g_server = ctx;

    LOG_INFO(COMPONENT, "Starting %s v%s", MAUM_APP_NAME, MAUM_APP_VERSION);

    int listen_fd = open_listen_socket(ctx->config.telnet_host, ctx->config.telnet_port);
    if (listen_fd < 0) {
        LOG_ERROR(COMPONENT, "%s", "Unable to create TELNET listener");
        return -1;
    }

    ctx->telnet_listen_fd = listen_fd;

    LOG_INFO(COMPONENT, "TELNET listening on %s:%u", ctx->config.telnet_host, ctx->config.telnet_port);

    if (ctx->config.enable_builtin_ssh) {
#ifdef MAUM_HAVE_LIBSSH
        LOG_INFO(COMPONENT, "%s", "Built-in SSH server support is enabled (not yet implemented in this build)");
#else
        LOG_WARN(COMPONENT, "%s", "Built-in SSH server requested but libssh is not available. Use --stdio mode with your SSH daemon.");
#endif
    }

    while (ctx->running) {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof(addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            if (!ctx->running) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            LOG_WARN(COMPONENT, "accept failed: %s", strerror(errno));
            continue;
        }

        char host[PEER_HOST_MAX];
        char service[PEER_SERVICE_MAX];
        if (getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof(host), service, sizeof(service),
                        NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
            strncpy(host, "unknown", sizeof(host) - 1);
            host[sizeof(host) - 1] = '\0';
            strncpy(service, "0", sizeof(service) - 1);
            service[sizeof(service) - 1] = '\0';
        }

        struct client_args *client = calloc(1, sizeof(*client));
        if (client == NULL) {
            LOG_WARN(COMPONENT, "%s", "Unable to allocate client args");
            close(client_fd);
            continue;
        }
        client->sessions = ctx->sessions;
        client->fd = client_fd;
        client->transport = SESSION_TRANSPORT_TELNET;
        snprintf(client->peer, sizeof(client->peer), "%s:%s", host, service);

        pthread_t thread;
        if (pthread_create(&thread, NULL, client_thread, client) != 0) {
            LOG_WARN(COMPONENT, "%s", "Failed to create client thread");
            close(client_fd);
            free(client);
            continue;
        }
        pthread_detach(thread);
    }

    close(listen_fd);
    ctx->telnet_listen_fd = -1;
    LOG_INFO(COMPONENT, "%s", "Server shutdown");
    return 0;
}

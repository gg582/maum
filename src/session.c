#include "session.h"

#include "log.h"

#include <ctype.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define COMPONENT "session"
#define USERNAME_MAX BOARD_AUTHOR_MAX

struct chat_client {
    FILE *out;
    session_transport_t transport;
    char username[USERNAME_MAX];
    char peer[64];
    struct chat_client *next;
};

struct session_manager {
    board_t *board;
    pthread_mutex_t lock;
    struct chat_client *chat_clients;
    char motd_path[256];
};

static void trim_line(char *line)
{
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
    size_t start = 0;
    while (line[start] != '\0' && isspace((unsigned char)line[start])) {
        start++;
    }
    if (start > 0) {
        memmove(line, line + start, strlen(line + start) + 1);
    }
}

static void send_text(FILE *out, const char *fmt, ...)
{
    if (out == NULL || fmt == NULL) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
    fflush(out);
}

static void send_line(FILE *out, const char *fmt, ...)
{
    if (out == NULL || fmt == NULL) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
    fputs("\r\n", out);
    fflush(out);
}

static int read_line(FILE *in, char *buffer, size_t size)
{
    if (fgets(buffer, (int)size, in) == NULL) {
        return -1;
    }
    trim_line(buffer);
    return 0;
}

static void send_motd(session_manager_t *manager, FILE *out)
{
    if (manager == NULL || out == NULL) {
        return;
    }

    FILE *file = fopen(manager->motd_path, "r");
    if (file == NULL) {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        trim_line(line);
        if (line[0] == '\0') {
            send_line(out, "");
        } else {
            send_line(out, "%s", line);
        }
    }

    fclose(file);
}

static const char *transport_label(session_transport_t transport)
{
    switch (transport) {
    case SESSION_TRANSPORT_TELNET:
        return "TELNET";
    case SESSION_TRANSPORT_SSH:
        return "SSH";
    case SESSION_TRANSPORT_STDIO:
        return "STDIO";
    default:
        return "UNKNOWN";
    }
}

session_manager_t *session_manager_create(const maum_config_t *config)
{
    if (config == NULL) {
        return NULL;
    }

    session_manager_t *manager = calloc(1, sizeof(*manager));
    if (manager == NULL) {
        return NULL;
    }

    manager->board = board_create(config->board_path);
    if (manager->board == NULL) {
        free(manager);
        return NULL;
    }

    if (pthread_mutex_init(&manager->lock, NULL) != 0) {
        board_destroy(manager->board);
        free(manager);
        return NULL;
    }

    strncpy(manager->motd_path, config->motd_path, sizeof(manager->motd_path) - 1);
    manager->motd_path[sizeof(manager->motd_path) - 1] = '\0';

    return manager;
}

void session_manager_destroy(session_manager_t *manager)
{
    if (manager == NULL) {
        return;
    }

    board_destroy(manager->board);
    pthread_mutex_destroy(&manager->lock);

    struct chat_client *client = manager->chat_clients;
    while (client != NULL) {
        struct chat_client *next = client->next;
        free(client);
        client = next;
    }

    free(manager);
}

static void chat_broadcast(session_manager_t *manager, const char *message)
{
    if (manager == NULL || message == NULL) {
        return;
    }

    if (pthread_mutex_lock(&manager->lock) != 0) {
        return;
    }

    struct chat_client *client = manager->chat_clients;
    while (client != NULL) {
        send_line(client->out, "%s", message);
        client = client->next;
    }

    pthread_mutex_unlock(&manager->lock);
}

static struct chat_client *chat_join(session_manager_t *manager,
                                     FILE *out,
                                     const char *username,
                                     session_transport_t transport,
                                     const char *peer)
{
    struct chat_client *client = calloc(1, sizeof(*client));
    if (client == NULL) {
        return NULL;
    }

    client->out = out;
    client->transport = transport;
    strncpy(client->username, username, sizeof(client->username) - 1);
    client->username[sizeof(client->username) - 1] = '\0';
    if (peer != NULL) {
        strncpy(client->peer, peer, sizeof(client->peer) - 1);
        client->peer[sizeof(client->peer) - 1] = '\0';
    }

    if (pthread_mutex_lock(&manager->lock) != 0) {
        free(client);
        return NULL;
    }

    client->next = manager->chat_clients;
    manager->chat_clients = client;

    pthread_mutex_unlock(&manager->lock);

    char notice[256];
    snprintf(notice, sizeof(notice), "[알림] %s (%s) 님이 입장했습니다.",
             client->username, transport_label(client->transport));
    chat_broadcast(manager, notice);

    return client;
}

static void chat_leave(session_manager_t *manager, struct chat_client *client)
{
    if (manager == NULL || client == NULL) {
        return;
    }

    if (pthread_mutex_lock(&manager->lock) != 0) {
        free(client);
        return;
    }

    struct chat_client **cursor = &manager->chat_clients;
    while (*cursor != NULL) {
        if (*cursor == client) {
            *cursor = client->next;
            break;
        }
        cursor = &(*cursor)->next;
    }

    pthread_mutex_unlock(&manager->lock);

    char notice[256];
    snprintf(notice, sizeof(notice), "[알림] %s 님이 퇴장했습니다.", client->username);
    chat_broadcast(manager, notice);

    free(client);
}

static void handle_chat(session_manager_t *manager,
                        FILE *in,
                        FILE *out,
                        const char *username,
                        session_transport_t transport,
                        const char *peer)
{
    struct chat_client *client = chat_join(manager, out, username, transport, peer);
    if (client == NULL) {
        send_line(out, "채팅방에 입장할 수 없습니다. 잠시 후 다시 시도해주세요.");
        return;
    }

    send_line(out, "채팅방에 입장했습니다. '/exit' 입력 시 나갑니다.");

    char buffer[BOARD_CONTENT_MAX];
    while (1) {
        send_text(out, "> ");
        if (read_line(in, buffer, sizeof(buffer)) != 0) {
            break;
        }
        if (strcmp(buffer, "/exit") == 0) {
            break;
        }
        if (buffer[0] == '\0') {
            continue;
        }
        for (char *p = buffer; *p != '\0'; ++p) {
            if (*p == '\r' || *p == '\n') {
                *p = ' ';
            }
        }

        char message[BOARD_CONTENT_MAX + 128];
        snprintf(message, sizeof(message), "[%s][%s] %s", transport_label(transport), username, buffer);
        chat_broadcast(manager, message);
    }

    chat_leave(manager, client);
    send_line(out, "채팅방을 떠났습니다.");
}

static void handle_board_list(session_manager_t *manager, FILE *out)
{
    board_post_t *posts = NULL;
    size_t count = 0;
    if (board_list(manager->board, &posts, &count) != 0) {
        send_line(out, "게시판을 불러오지 못했습니다.");
        return;
    }

    if (count == 0) {
        send_line(out, "등록된 게시물이 없습니다. 첫 번째 글을 남겨보세요!");
        free(posts);
        return;
    }

    send_line(out, "총 %zu개의 게시물이 있습니다:", count);
    for (size_t i = 0; i < count; ++i) {
        send_line(out, "[%u] %s — %s", posts[i].id, posts[i].author, posts[i].timestamp);
        send_line(out, "    %s", posts[i].content);
    }

    free(posts);
}

static void sanitize_content(char *content)
{
    for (char *p = content; *p != '\0'; ++p) {
        if (*p == '|' || *p == '\r' || *p == '\n') {
            *p = ' ';
        }
    }
}

static void handle_board_add(session_manager_t *manager, FILE *in, FILE *out, const char *username)
{
    send_text(out, "게시물 내용을 입력하세요 (한 줄): ");
    char buffer[BOARD_CONTENT_MAX];
    if (read_line(in, buffer, sizeof(buffer)) != 0) {
        send_line(out, "입력을 받지 못했습니다.");
        return;
    }
    sanitize_content(buffer);
    if (buffer[0] == '\0') {
        send_line(out, "내용이 비어 있습니다.");
        return;
    }

    board_post_t post;
    if (board_add(manager->board, username, buffer, &post) != 0) {
        send_line(out, "게시물을 저장하는데 실패했습니다.");
        return;
    }

    send_line(out, "[#%u] 등록 완료 (%s)", post.id, post.timestamp);
}

static void handle_board_delete(session_manager_t *manager, FILE *in, FILE *out, const char *username)
{
    send_text(out, "삭제할 게시물 번호: ");
    char buffer[32];
    if (read_line(in, buffer, sizeof(buffer)) != 0) {
        send_line(out, "입력을 받지 못했습니다.");
        return;
    }
    unsigned long id = strtoul(buffer, NULL, 10);
    if (id == 0) {
        send_line(out, "올바른 번호를 입력하세요.");
        return;
    }

    int not_owner = 0;
    int result = board_remove(manager->board, (unsigned int)id, username, &not_owner);
    if (result == 0) {
        send_line(out, "게시물이 삭제되었습니다.");
    } else if (result == 1) {
        send_line(out, "해당 번호의 게시물이 없습니다.");
    } else if (result == 2 && not_owner) {
        send_line(out, "본인이 작성한 글만 삭제할 수 있습니다.");
    } else {
        send_line(out, "게시물을 삭제하는데 실패했습니다.");
    }
}

static int prompt_username(FILE *in, FILE *out, char *username, size_t size)
{
    for (int attempts = 0; attempts < 3; ++attempts) {
        send_text(out, "사용할 닉네임을 입력하세요: ");
        if (read_line(in, username, size) != 0) {
            return -1;
        }
        sanitize_content(username);
        if (username[0] == '\0') {
            send_line(out, "닉네임은 비워둘 수 없습니다.");
            continue;
        }
        if (strlen(username) >= size) {
            username[size - 1] = '\0';
        }
        return 0;
    }
    return -1;
}

void session_manager_run(session_manager_t *manager,
                         session_transport_t transport,
                         FILE *input,
                         FILE *output,
                         const char *peer_identity)
{
    if (manager == NULL || input == NULL || output == NULL) {
        return;
    }

    setvbuf(output, NULL, _IONBF, 0);

    send_line(output, "마음 (Maum) BBS에 오신 것을 환영합니다!");
    if (peer_identity != NULL) {
        send_line(output, "접속: %s [%s]", peer_identity, transport_label(transport));
    } else {
        send_line(output, "접속: %s", transport_label(transport));
    }
    send_line(output, "────────────────────────────────────");
    send_motd(manager, output);
    send_line(output, "────────────────────────────────────");

    char username[USERNAME_MAX];
    if (prompt_username(input, output, username, sizeof(username)) != 0) {
        send_line(output, "닉네임 설정에 실패했습니다. 연결을 종료합니다.");
        return;
    }

    send_line(output, "환영합니다, %s님!", username);

    char choice[16];
    int running = 1;
    while (running) {
        send_line(output, "");
        send_line(output, "┌──────────────────────────────┐");
        send_line(output, "│ 1) 실시간 채팅 참여           │");
        send_line(output, "│ 2) 게시물 목록 보기           │");
        send_line(output, "│ 3) 새 게시물 등록             │");
        send_line(output, "│ 4) 내 게시물 삭제             │");
        send_line(output, "│ 5) 종료                       │");
        send_line(output, "└──────────────────────────────┘");
        send_text(output, "메뉴 선택 (1-5): ");
        if (read_line(input, choice, sizeof(choice)) != 0) {
            break;
        }

        if (strcmp(choice, "1") == 0) {
            handle_chat(manager, input, output, username, transport, peer_identity);
        } else if (strcmp(choice, "2") == 0) {
            handle_board_list(manager, output);
        } else if (strcmp(choice, "3") == 0) {
            handle_board_add(manager, input, output, username);
        } else if (strcmp(choice, "4") == 0) {
            handle_board_delete(manager, input, output, username);
        } else if (strcmp(choice, "5") == 0 || strcasecmp(choice, "q") == 0) {
            running = 0;
        } else {
            send_line(output, "알 수 없는 선택입니다.");
        }
    }

    send_line(output, "안녕히 가세요, %s님!", username);
}

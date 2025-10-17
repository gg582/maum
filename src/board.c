#include "board.h"

#include "log.h"

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>

#define COMPONENT "board"

struct board {
    char path[256];
    char pin_path[256];
    pthread_mutex_t lock;
    unsigned int next_id;
};

static int ensure_directory_exists(const char *path)
{
    char buffer[512];
    strncpy(buffer, path, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *slash = strrchr(buffer, '/');
    if (slash == NULL) {
        return 0;
    }
    *slash = '\0';
    if (buffer[0] == '\0') {
        return 0;
    }

    struct stat st;
    if (stat(buffer, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0;
        }
        LOG_ERROR(COMPONENT, "'%s' exists but is not a directory", buffer);
        return -1;
    }

    if (mkdir(buffer, 0755) != 0 && errno != EEXIST) {
        LOG_ERROR(COMPONENT, "Failed to create directory '%s': %s", buffer, strerror(errno));
        return -1;
    }
    return 0;
}

static void update_next_id(board_t *board)
{
    FILE *file = fopen(board->path, "r");
    if (file == NULL) {
        board->next_id = 1;
        return;
    }

    unsigned int max_id = 0;
    char line[BOARD_AUTHOR_MAX + BOARD_TIMESTAMP_MAX + BOARD_CONTENT_MAX + 32];
    while (fgets(line, sizeof(line), file) != NULL) {
        unsigned long value = strtoul(line, NULL, 10);
        if (value > max_id) {
            max_id = (unsigned int)value;
        }
    }

    fclose(file);
    board->next_id = max_id + 1;
}

static void build_pin_path(board_t *board, const char *path)
{
    snprintf(board->pin_path, sizeof(board->pin_path), "%s.pin", path);
}

board_t *board_create(const char *path)
{
    if (path == NULL) {
        return NULL;
    }

    if (ensure_directory_exists(path) != 0) {
        return NULL;
    }

    board_t *board = calloc(1, sizeof(*board));
    if (board == NULL) {
        return NULL;
    }

    strncpy(board->path, path, sizeof(board->path) - 1);
    board->path[sizeof(board->path) - 1] = '\0';
    build_pin_path(board, board->path);

    if (pthread_mutex_init(&board->lock, NULL) != 0) {
        free(board);
        return NULL;
    }

    FILE *file = fopen(board->path, "a+");
    if (file == NULL) {
        LOG_ERROR(COMPONENT, "Unable to open board storage '%s': %s", path, strerror(errno));
        pthread_mutex_destroy(&board->lock);
        free(board);
        return NULL;
    }
    fclose(file);

    FILE *pin_file = fopen(board->pin_path, "a+");
    if (pin_file == NULL) {
        LOG_ERROR(COMPONENT, "Unable to open board credential storage '%s': %s", board->pin_path,
                  strerror(errno));
        pthread_mutex_destroy(&board->lock);
        free(board);
        return NULL;
    }
    fclose(pin_file);

    update_next_id(board);
    return board;
}

int board_author_pin_load(board_t *board, const char *author, char *pin, size_t size)
{
    if (board == NULL || author == NULL || pin == NULL || size == 0) {
        return -1;
    }

    if (pthread_mutex_lock(&board->lock) != 0) {
        return -1;
    }

    FILE *file = fopen(board->pin_path, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    int result = 1;
    char line[BOARD_AUTHOR_MAX + BOARD_PIN_MAX + 8];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *sep = strchr(line, '|');
        if (sep == NULL) {
            continue;
        }
        *sep = '\0';
        char *stored_pin = sep + 1;
        size_t len = strlen(stored_pin);
        while (len > 0 && (stored_pin[len - 1] == '\n' || stored_pin[len - 1] == '\r')) {
            stored_pin[len - 1] = '\0';
            len--;
        }
        if (strcmp(line, author) == 0) {
            strncpy(pin, stored_pin, size - 1);
            pin[size - 1] = '\0';
            result = 0;
            break;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&board->lock);
    return result;
}

int board_author_pin_store(board_t *board, const char *author, const char *pin)
{
    if (board == NULL || author == NULL || pin == NULL) {
        return -1;
    }

    if (pthread_mutex_lock(&board->lock) != 0) {
        return -1;
    }

    FILE *file = fopen(board->pin_path, "r");
    char temp_path[sizeof(((board_t *)0)->pin_path) + 4];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", board->pin_path);
    FILE *temp = fopen(temp_path, "w");
    if (temp == NULL) {
        if (file != NULL) {
            fclose(file);
        }
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    bool updated = false;
    if (file != NULL) {
        char line[BOARD_AUTHOR_MAX + BOARD_PIN_MAX + 8];
        while (fgets(line, sizeof(line), file) != NULL) {
            char *sep = strchr(line, '|');
            if (sep == NULL) {
                continue;
            }
            *sep = '\0';
            char *existing_pin = sep + 1;
            size_t len = strlen(existing_pin);
            while (len > 0 && (existing_pin[len - 1] == '\n' || existing_pin[len - 1] == '\r')) {
                existing_pin[len - 1] = '\0';
                len--;
            }
            if (strcmp(line, author) == 0) {
                fprintf(temp, "%s|%s\n", author, pin);
                updated = true;
            } else {
                fprintf(temp, "%s|%s\n", line, existing_pin);
            }
        }
        fclose(file);
    }

    if (!updated) {
        fprintf(temp, "%s|%s\n", author, pin);
    }

    fflush(temp);
    fclose(temp);

    if (rename(temp_path, board->pin_path) != 0) {
        unlink(temp_path);
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    pthread_mutex_unlock(&board->lock);
    return 0;
}

void board_destroy(board_t *board)
{
    if (board == NULL) {
        return;
    }
    pthread_mutex_destroy(&board->lock);
    free(board);
}

static int parse_line(const char *line, board_post_t *post)
{
    char buffer[BOARD_AUTHOR_MAX + BOARD_TIMESTAMP_MAX + BOARD_CONTENT_MAX + 16];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *saveptr = NULL;
    char *token = strtok_r(buffer, "|", &saveptr);
    if (token == NULL) {
        return -1;
    }
    post->id = (unsigned int)strtoul(token, NULL, 10);

    token = strtok_r(NULL, "|", &saveptr);
    if (token == NULL) {
        return -1;
    }
    strncpy(post->timestamp, token, sizeof(post->timestamp) - 1);
    post->timestamp[sizeof(post->timestamp) - 1] = '\0';

    token = strtok_r(NULL, "|", &saveptr);
    if (token == NULL) {
        return -1;
    }
    strncpy(post->author, token, sizeof(post->author) - 1);
    post->author[sizeof(post->author) - 1] = '\0';

    token = strtok_r(NULL, "", &saveptr);
    if (token == NULL) {
        post->content[0] = '\0';
    } else {
        strncpy(post->content, token, sizeof(post->content) - 1);
        post->content[sizeof(post->content) - 1] = '\0';
    }

    size_t len = strlen(post->content);
    while (len > 0 && (post->content[len - 1] == '\r' || post->content[len - 1] == '\n')) {
        post->content[len - 1] = '\0';
        len--;
    }
    return 0;
}

int board_list(board_t *board, board_post_t **posts, size_t *count)
{
    if (board == NULL || posts == NULL || count == NULL) {
        return -1;
    }

    *posts = NULL;
    *count = 0;

    if (pthread_mutex_lock(&board->lock) != 0) {
        return -1;
    }

    FILE *file = fopen(board->path, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    size_t capacity = 8;
    board_post_t *items = calloc(capacity, sizeof(*items));
    if (items == NULL) {
        fclose(file);
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    char line[BOARD_AUTHOR_MAX + BOARD_TIMESTAMP_MAX + BOARD_CONTENT_MAX + 32];
    while (fgets(line, sizeof(line), file) != NULL) {
        board_post_t post;
        if (parse_line(line, &post) != 0) {
            continue;
        }
        if (*count >= capacity) {
            capacity *= 2;
            board_post_t *tmp = realloc(items, capacity * sizeof(*items));
            if (tmp == NULL) {
                free(items);
                fclose(file);
                pthread_mutex_unlock(&board->lock);
                return -1;
            }
            items = tmp;
        }
        items[*count] = post;
        (*count)++;
    }

    fclose(file);
    pthread_mutex_unlock(&board->lock);

    *posts = items;
    return 0;
}

static void build_timestamp(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    strftime(buffer, size, "%Y-%m-%d %H:%M", &tm_now);
}

int board_add(board_t *board, const char *author, const char *content, board_post_t *out_post)
{
    if (board == NULL || author == NULL || content == NULL) {
        return -1;
    }
    if (strlen(author) == 0 || strlen(author) >= BOARD_AUTHOR_MAX) {
        return -1;
    }
    if (strlen(content) == 0 || strlen(content) >= BOARD_CONTENT_MAX) {
        return -1;
    }

    if (pthread_mutex_lock(&board->lock) != 0) {
        return -1;
    }

    FILE *file = fopen(board->path, "a");
    if (file == NULL) {
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    unsigned int id = board->next_id++;
    char timestamp[BOARD_TIMESTAMP_MAX];
    build_timestamp(timestamp, sizeof(timestamp));

    if (fprintf(file, "%u|%s|%s|%s\n", id, timestamp, author, content) < 0) {
        fclose(file);
        pthread_mutex_unlock(&board->lock);
        return -1;
    }
    fflush(file);
    fclose(file);

    if (out_post != NULL) {
        out_post->id = id;
        strncpy(out_post->author, author, sizeof(out_post->author) - 1);
        out_post->author[sizeof(out_post->author) - 1] = '\0';
        strncpy(out_post->timestamp, timestamp, sizeof(out_post->timestamp) - 1);
        out_post->timestamp[sizeof(out_post->timestamp) - 1] = '\0';
        strncpy(out_post->content, content, sizeof(out_post->content) - 1);
        out_post->content[sizeof(out_post->content) - 1] = '\0';
    }

    pthread_mutex_unlock(&board->lock);
    return 0;
}

int board_remove(board_t *board, unsigned int id, const char *requester, int *not_owner)
{
    if (board == NULL) {
        return -1;
    }

    if (pthread_mutex_lock(&board->lock) != 0) {
        return -1;
    }

    FILE *file = fopen(board->path, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    char temp_path[sizeof(((board_t *)0)->path) + 4];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", board->path);
    FILE *temp = fopen(temp_path, "w");
    if (temp == NULL) {
        fclose(file);
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    char line[BOARD_AUTHOR_MAX + BOARD_TIMESTAMP_MAX + BOARD_CONTENT_MAX + 32];
    int found = 0;
    int owner_mismatch = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        board_post_t post;
        if (parse_line(line, &post) != 0) {
            continue;
        }
        if (post.id == id) {
            found = 1;
            if (requester != NULL && requester[0] != '\0' && strcmp(post.author, requester) != 0) {
                owner_mismatch = 1;
                fputs(line, temp);
            }
            continue;
        }
        fputs(line, temp);
    }

    fclose(file);
    fflush(temp);
    fclose(temp);

    if (!found) {
        unlink(temp_path);
        pthread_mutex_unlock(&board->lock);
        if (not_owner != NULL) {
            *not_owner = 0;
        }
        return 1;
    }

    if (owner_mismatch) {
        unlink(temp_path);
        if (not_owner != NULL) {
            *not_owner = 1;
        }
        pthread_mutex_unlock(&board->lock);
        return 2;
    }

    if (rename(temp_path, board->path) != 0) {
        LOG_ERROR(COMPONENT, "Failed to replace board storage: %s", strerror(errno));
        unlink(temp_path);
        pthread_mutex_unlock(&board->lock);
        return -1;
    }

    if (not_owner != NULL) {
        *not_owner = 0;
    }

    pthread_mutex_unlock(&board->lock);
    return 0;
}

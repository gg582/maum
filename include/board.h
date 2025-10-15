#ifndef BOARD_H
#define BOARD_H

#include <stddef.h>

#define BOARD_AUTHOR_MAX 32
#define BOARD_CONTENT_MAX 512
#define BOARD_TIMESTAMP_MAX 32

typedef struct board board_t;

typedef struct {
    unsigned int id;
    char author[BOARD_AUTHOR_MAX];
    char timestamp[BOARD_TIMESTAMP_MAX];
    char content[BOARD_CONTENT_MAX];
} board_post_t;

board_t *board_create(const char *path);
void board_destroy(board_t *board);

int board_list(board_t *board, board_post_t **posts, size_t *count);
int board_add(board_t *board, const char *author, const char *content, board_post_t *out_post);
int board_remove(board_t *board, unsigned int id, const char *requester, int *not_owner);

#endif // BOARD_H

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stddef.h>

#define CONFIG_MAX_HOST_LEN 128

typedef struct {
    char ssh_host[CONFIG_MAX_HOST_LEN];
    unsigned short ssh_port;
    char telnet_host[CONFIG_MAX_HOST_LEN];
    unsigned short telnet_port;
    char motd_path[256];
    char board_path[256];
    char host_key_path[256];
    bool enable_builtin_ssh;
} maum_config_t;

void config_init(maum_config_t *config);
int config_load(maum_config_t *config, const char *path);

#endif // CONFIG_H

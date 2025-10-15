#include "config.h"

#include "log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define COMPONENT "config"

static void trim_whitespace(char *line)
{
    size_t len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1])) {
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

void config_init(maum_config_t *config)
{
    if (config == NULL) {
        return;
    }
    memset(config->ssh_host, 0, sizeof(config->ssh_host));
    memset(config->telnet_host, 0, sizeof(config->telnet_host));
    memset(config->motd_path, 0, sizeof(config->motd_path));
    memset(config->board_path, 0, sizeof(config->board_path));
    memset(config->host_key_path, 0, sizeof(config->host_key_path));

    strncpy(config->ssh_host, "0.0.0.0", sizeof(config->ssh_host) - 1);
    config->ssh_port = 2222;
    strncpy(config->telnet_host, "0.0.0.0", sizeof(config->telnet_host) - 1);
    config->telnet_port = 2323;
    strncpy(config->motd_path, "motd.txt", sizeof(config->motd_path) - 1);
    strncpy(config->board_path, "data/posts.db", sizeof(config->board_path) - 1);
    strncpy(config->host_key_path, "data/maum_host_ed25519", sizeof(config->host_key_path) - 1);
    config->enable_builtin_ssh = false;
}

static bool parse_bool(const char *value)
{
    if (value == NULL) {
        return false;
    }
    if (strcasecmp(value, "true") == 0 || strcasecmp(value, "yes") == 0 || strcmp(value, "1") == 0) {
        return true;
    }
    return false;
}

static int parse_line(maum_config_t *config, const char *key, const char *value)
{
    if (strcmp(key, "ssh_host") == 0) {
        strncpy(config->ssh_host, value, sizeof(config->ssh_host) - 1);
        return 0;
    }
    if (strcmp(key, "ssh_port") == 0) {
        config->ssh_port = (unsigned short)strtoul(value, NULL, 10);
        return 0;
    }
    if (strcmp(key, "telnet_host") == 0) {
        strncpy(config->telnet_host, value, sizeof(config->telnet_host) - 1);
        return 0;
    }
    if (strcmp(key, "telnet_port") == 0) {
        config->telnet_port = (unsigned short)strtoul(value, NULL, 10);
        return 0;
    }
    if (strcmp(key, "motd_path") == 0) {
        strncpy(config->motd_path, value, sizeof(config->motd_path) - 1);
        return 0;
    }
    if (strcmp(key, "board_path") == 0) {
        strncpy(config->board_path, value, sizeof(config->board_path) - 1);
        return 0;
    }
    if (strcmp(key, "host_key_path") == 0) {
        strncpy(config->host_key_path, value, sizeof(config->host_key_path) - 1);
        return 0;
    }
    if (strcmp(key, "enable_builtin_ssh") == 0) {
        config->enable_builtin_ssh = parse_bool(value);
        return 0;
    }
    LOG_WARN(COMPONENT, "Unknown configuration key '%s'", key);
    return -1;
}

int config_load(maum_config_t *config, const char *path)
{
    if (config == NULL || path == NULL) {
        return -1;
    }

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        LOG_WARN(COMPONENT, "Unable to open config file '%s', using defaults", path);
        return -1;
    }

    char line[256];
    unsigned long lineno = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        lineno++;
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        char *equals = strchr(line, '=');
        if (equals == NULL) {
            LOG_WARN(COMPONENT, "Ignoring malformed line %lu", lineno);
            continue;
        }
        *equals = '\0';
        char *key = line;
        char *value = equals + 1;
        trim_whitespace(key);
        trim_whitespace(value);
        parse_line(config, key, value);
    }

    fclose(file);
    return 0;
}

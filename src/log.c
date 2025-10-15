#include "log.h"

#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

static log_level_t current_level = LOG_LEVEL_INFO;

static const char *level_to_string(log_level_t level)
{
    switch (level) {
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_INFO:
        return "INFO";
    case LOG_LEVEL_WARN:
        return "WARN";
    case LOG_LEVEL_ERROR:
        return "ERROR";
    }
    return "UNKNOWN";
}

void log_set_level(log_level_t level)
{
    current_level = level;
}

void log_message(log_level_t level, const char *component, const char *fmt, ...)
{
    if (level < current_level) {
        return;
    }

    time_t now = time(NULL);
    struct tm *tm_snapshot = localtime(&now);
    if (tm_snapshot == NULL) {
        return;
    }

    char timestamp[32];
    if (strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_snapshot) == 0) {
        return;
    }

    FILE *output = (level >= LOG_LEVEL_WARN) ? stderr : stdout;
    fprintf(output, "%s [%s] %s: ", timestamp, level_to_string(level), component);

    va_list args;
    va_start(args, fmt);
    vfprintf(output, fmt, args);
    va_end(args);

    fputc('\n', output);
}

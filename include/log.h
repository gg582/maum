#ifndef LOG_H
#define LOG_H

#include <stdio.h>

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} log_level_t;

void log_set_level(log_level_t level);
void log_message(log_level_t level, const char *component, const char *fmt, ...);

#define LOG_DEBUG(component, fmt, ...) log_message(LOG_LEVEL_DEBUG, component, fmt, ##__VA_ARGS__)
#define LOG_INFO(component, fmt, ...)  log_message(LOG_LEVEL_INFO, component, fmt, ##__VA_ARGS__)
#define LOG_WARN(component, fmt, ...)  log_message(LOG_LEVEL_WARN, component, fmt, ##__VA_ARGS__)
#define LOG_ERROR(component, fmt, ...) log_message(LOG_LEVEL_ERROR, component, fmt, ##__VA_ARGS__)

#endif // LOG_H

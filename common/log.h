#ifndef M_LOG_H
#define M_LOG_H

enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_DEBUG,
};

extern int log_level;

int log_init(void);
void log_release(void);
void log_trace_level(int level, const char * fmt, ...);
void log_set_verbose();

#define log_warn(fmt, ...)      log_trace_level(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define log_trace(fmt, ...)     log_trace_level(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define log_verbose(fmt, ...)   log_trace_level(LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)     log_trace_level(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
void log_flush();

#endif

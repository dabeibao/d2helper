#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include "log.h"

#define LOG_NAME        "d2helper.log"

static FILE * log_file = NULL;
static int log_level = LOG_LEVEL_INFO;

int log_init(void)
{
    FILE *      fp = fopen(LOG_NAME, "w");

    if (fp == NULL) {
        fprintf(stderr, "Failed to open %s for write\n", LOG_NAME);
        return -1;
    }

    log_file = fp;
    return 0;
}

void log_release(void)
{
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_set_verbose(void)
{
    log_level = LOG_LEVEL_VERBOSE;
}

static void log_vtrace(const char *fmt, va_list ap)
{
    SYSTEMTIME  local_time;

    if (log_file == NULL) {
        return;
    }

    GetLocalTime(&local_time);

    char buf[256];
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (n <= 0 || n >= sizeof(buf)) {
        buf[sizeof(buf) - 1] = '\0';
    } else {
        if (buf[n - 1] == '\n') {
            buf[n - 1] = '\0';
        }
    }

    fprintf(log_file, "%02d:%02d:%02d.%03d %s\n",
            local_time.wHour, local_time.wMinute, local_time.wSecond, local_time.wMilliseconds,
            buf);
    log_flush();
}

void log_trace_level(int level, const char * fmt, ...)
{
    va_list     ap;

    if (level > log_level) {
        return;
    }

    va_start(ap, fmt);
    log_vtrace(fmt, ap);
    va_end(ap);
}

void log_flush(void)
{
    fflush(log_file);
}

//
// Created on 08.03.2022.
//

#ifndef _PI_MSDP_LOG_H_
#define _PI_MSDP_LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/**
 * /brief
 * A small wrapper type for the file-type
 */
typedef FILE msdp_log_t;

typedef struct {
    va_list fmt_parameters;
    void *ref_file;

    const char *code_file;
    const char *fmt;

    struct tm *time;

    int line;
    int level;
} log_event_t;

enum { LOG_TRACE = 0, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

msdp_log_t *log_init(const char *path);

#define log_trace(...) log_internal(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_internal(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_internal(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_internal(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_internal(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_internal(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void log_internal(int level, const char *file, int line, const char *fmt, ...);

#endif //_PI_MSDP_LOG_H_

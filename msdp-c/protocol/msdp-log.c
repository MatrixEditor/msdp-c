//
// Created by Leonard on 08.03.2022.
//

#include "msdp-log.h"

static const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static void std_out_print(log_event_t *event);
static void std_out_file(log_event_t *event);

static msdp_log_t *rel_log = NULL;

msdp_log_t *log_init(const char *path) {
  if (rel_log) return rel_log;

  msdp_log_t *l = fopen(path, "w");
  if (l != NULL) {
    rel_log = l;
  }
  return rel_log;
}

void log_internal(int level, const char *file, int line, const char *fmt, ...) {
  log_event_t event = {
          .code_file = file,
          .fmt = fmt,
          .line = line,
          .level = level
  };

  if (!event.time) {
    time_t t = time(NULL);
    event.time = localtime(&t);
  }

  va_start(event.fmt_parameters, fmt);
  if (rel_log) {
    event.ref_file = rel_log;
    std_out_file(&event);
  }
  event.ref_file = stderr;
  std_out_print(&event);
  va_end(event.fmt_parameters);
}

static void std_out_print(log_event_t *event) {
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", event->time)] = '\0';
  fprintf(event->ref_file, "%s %-5s %s:%d: ",
          buf, level_strings[event->level], event->code_file, event->line);
  vfprintf(event->ref_file, event->fmt, event->fmt_parameters);
  fprintf(event->ref_file, "\n");
  fflush(event->ref_file);
}

static void std_out_file(log_event_t *event) {
  char buf[64];
  buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", event->time)] = '\0';
  fprintf(event->ref_file, "%s %-5s %s:%d: ",
          buf, level_strings[event->level], event->code_file, event->line);
  vfprintf(event->ref_file, event->fmt, event->fmt_parameters);
  fprintf(event->ref_file, "\n");
  fflush(event->ref_file);
}
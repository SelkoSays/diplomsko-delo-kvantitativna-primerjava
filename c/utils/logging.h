#ifndef LOGGING_H
#define LOGGING_H

#define DEBUG_STR ANSI_YELLOW "[DEBUG]" ANSI_RESET
#define INFO_STR ANSI_CYAN "[INFO]" ANSI_RESET
#define WARN_STR ANSI_YELLOW "[WARN]" ANSI_RESET
#define ERROR_STR ANSI_RED "[ERROR]" ANSI_RESET

#if defined(LOG_FILE_COLOR)
#define FDEBUG_STR DEBUG_STR
#define FINFO_STR INFO_STR
#define FWARN_STR WARN_STR
#define FERROR_STR ERROR_STR
#else
#define FDEBUG_STR "[DEBUG]"
#define FINFO_STR "[INFO]"
#define FWARN_STR "[WARN]"
#define FERROR_STR "[ERROR]"
#endif // LOG_FILE_COLOR

#if defined(LOG_LEVEL)

#include "common.h"

#define log_debug(...) eprintln(DEBUG_STR " " __VA_ARGS__)
#define log_info(...) println(INFO_STR " " __VA_ARGS__)
#define log_warn(...) eprintln(WARN_STR " " __VA_ARGS__)
#define log_error(...) eprintln(ERROR_STR " " __VA_ARGS__)

#define flog_debug(file, ...) fprintln(file, FDEBUG_STR " " __VA_ARGS__)
#define flog_info(file, ...) fprintln(file, FINFO_STR " " __VA_ARGS__)
#define flog_warn(file, ...) fprintln(file, FWARN_STR " " __VA_ARGS__)
#define flog_error(file, ...) fprintln(file, FERROR_STR " " __VA_ARGS__)

#if LOG_LEVEL > 0
#undef log_debug
#define log_debug(...)
#undef flog_debug
#define flog_debug(...)
#endif

#if LOG_LEVEL > 1
#undef log_info
#define log_info(...)
#undef flog_info
#define flog_info(...)
#endif

#if LOG_LEVEL > 2
#undef log_warn
#define log_warn(...)
#undef flog_warn
#define flog_warn(...)
#endif

#else

#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)

#define flog_debug(...)
#define flog_info(...)
#define flog_warn(...)
#define flog_error(...)

#endif // LOG_LEVEL

#endif // LOGGING_H

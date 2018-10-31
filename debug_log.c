#include "debug_log.h"

void debug_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputs("\n", stderr);
    fflush(stderr);
    va_end(args);
}

#ifndef MINI_UTIL_H
#define MINI_UTIL_H

#include <stddef.h>
#include <stdarg.h>

void fail(const char *fmt, ...);
char *read_whole_file(char *filepath);
int str_to_int(const char *s, size_t length);

#endif

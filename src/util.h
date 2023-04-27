#ifndef MINI_UTIL_H
#define MINI_UTIL_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define MAX(x, y) (x > y) ? x : y
#define MIN(x, y) (x > y) ? y : x

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_RESET   "\x1b[0m"

#define LOG_INFO(fmt, ...)  fprintf(stderr, ANSI_GREEN "[INFO] " ANSI_RESET fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  fprintf(stderr, ANSI_YELLOW "[WARN] " ANSI_RESET fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) fprintf(stderr, ANSI_RED "[ERROR] " ANSI_RESET fmt "\n", ##__VA_ARGS__)

void fatal(const char *fmt, ...);

uint64_t hash(const char *s);
uint64_t hash_n(uint8_t *data, size_t size);

int str_to_int(const char *s, size_t length);
char *aprintf(const char *fmt, ...);
char *rand_str(size_t length);

#endif

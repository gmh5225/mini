#ifndef MINI_UTIL_H
#define MINI_UTIL_H

#include "lex.h"

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define MAX(x, y) (x > y) ? x : y
#define MIN(x, y) (x > y) ? y : x

void error(const char *fmt, ...);
void error_at(int line, int col, const char *fmt, ...);
void error_at_token(Token *t, const char *fmt, ...);
void error_with_context(char *loc, const char *fmt, ...);

uint64_t hash(const char *s);
int str_to_int(const char *s, size_t length);
char *rand_str(size_t length);

#endif

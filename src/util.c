#include "util.h"

#include <stdio.h>
#include <stdlib.h>

void error(const char *fmt, ...) {
    fprintf(stderr, "mini: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void error_at(int line, int col, const char *fmt, ...) {
    fprintf(stderr, "mini: at line %d, col %d:\n -> ", line, col);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void error_at_token(Token *t, const char *fmt, ...) {
    fprintf(stderr, "mini: at line %d, col %d: on token %s\n -> ", 
            t->line, t->col, token_as_str(t->kind));
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

char *rand_str(size_t length) {
    static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define ALPHABET_LENGTH sizeof(alphabet)/sizeof(alphabet[0])
    char *result = calloc(length + 1, sizeof(char));
    for (size_t i = 0; i < length; i++) {
        size_t index = rand() % ALPHABET_LENGTH;
        result[i] = alphabet[index];
    }
    result[length] = 0;
    return result;
#undef ALPHABET_LENGTH
}

int str_to_int(const char *s, size_t length) {
    int n = 0;
    for (size_t i = 0; i < length; i++) {
        n = n * 10 + (s[i] - '0');
    }
    return n;
}

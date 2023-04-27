#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void fatal(const char *fmt, ...)
{
    fprintf(stderr, "mini: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

int str_to_int(const char *s, size_t length)
{
    int n = 0;
    for (size_t i = 0; i < length; i++) {
        n = n * 10 + (s[i] - '0');
    }
    return n;
}

uint64_t hash(const char *s)
{
    uint64_t hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

uint64_t hash_n(uint8_t *data, size_t size)
{
    const uint64_t FNV_PRIME = 0x100000001b3;
    const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325;

    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

char *aprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    char *str = calloc(size + 1, sizeof(char));
    va_start(args, fmt);
    vsnprintf(str, size + 1, fmt, args);
    va_end(args);
    return str;
}

char *rand_str(size_t length)
{
    static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define ALPHABET_LENGTH sizeof(alphabet)/sizeof(alphabet[0])
    char *str = calloc(length + 1, sizeof(char));
    for (size_t i = 0; i < length; i++) {
        size_t index = rand() % ALPHABET_LENGTH;
        str[i] = alphabet[index];
    }
    str[length] = 0;
    return str;
#undef ALPHABET_LENGTH
}

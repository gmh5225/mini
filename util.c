#include "util.h"
#include <stdio.h>
#include <stdlib.h>

int str_to_int(const char *s, size_t length) {
    int n = 0;
    for (size_t i = 0; i < length; i++) {
        n = n * 10 + (s[i] - '0');
    }
    return n;
}

char *read_whole_file(char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fail("couldn't open file '%s'", filepath);
    }

    fseek(f, 0L, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);

    char *buf = malloc(sizeof(char) * (file_size + 1));
    if (!buf) {
        fail("couldn't malloc buffer to read source code");
    }

    size_t nread = fread(buf, sizeof(char), file_size, f);
    if (nread != file_size) {
        fail("only read %zu bytes from file '%s' with size %zu", nread, filepath, file_size);
    }
    buf[nread] = 0; // This null terminator is really important...
    fclose(f);

    return buf;
}

void fail(const char *fmt, ...) {
    fprintf(stderr, "mini: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

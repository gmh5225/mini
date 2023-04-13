#include "codegen.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void code_buffer_init(CodeBuffer *code) {
    code->buffer = calloc(DEFAULT_TARGET_CODE_CAPACITY, sizeof(char));
    code->code_length = 0;
    code->code_capacity = DEFAULT_TARGET_CODE_CAPACITY;
}

void code_buffer_write_bytes(CodeBuffer *code, char *bytes, size_t length) {
    if (code->code_length + length >= code->code_capacity) {
        code->code_capacity <<= 1;
        void *tmp = realloc(code->buffer, sizeof(char) * code->code_capacity);
        code->buffer = tmp;
    }
    memcpy(code->buffer + code->code_length, bytes, length);
    code->code_length += length;
}

void code_buffer_write_to_file(CodeBuffer *code, char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        error("couldn't open file: `%s`", filename);
    }

    size_t nwritten = fwrite(code->buffer, sizeof(char), code->code_length, f);
    if (nwritten != code->code_length) {
#ifdef DEBUG
        error("only wrote %zu/%zu bytes to file `%s`",
                nwritten, code->code_length, filename);
#endif
    }
    fclose(f);
}

void code_buffer_free(CodeBuffer *code) {
    if (code->buffer) {
        free(code->buffer);
        code->buffer = NULL;
    }
    code->code_length = code->code_capacity = 0;
}

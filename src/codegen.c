#include "minigen/codegen.h"
#include "minigen/errors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct minigen_code_buffer {
    CodeTarget target;
    char *buffer;
    size_t code_length;
    size_t code_capacity;
};

void minigen_code_buffer_init(CodeBuffer *code, CodeTarget target) {
    code->target = target;
    code->buffer = calloc(DEFAULT_TARGET_CODE_CAPACITY, sizeof(char));
    code->code_length = 0;
    code->code_capacity = DEFAULT_TARGET_CODE_CAPACITY;
}

void minigen_code_buffer_write_bytes(CodeBuffer *code, char *bytes, size_t length) {
    if (code->code_length + length >= code->code_capacity) {
        code->code_capacity <<= 1;
        void *tmp = realloc(code->buffer, sizeof(char) * code->code_capacity);
        code->buffer = tmp;
    }
    memcpy(code->buffer + code->code_length, bytes, length);
    code->code_length += length;
}

int minigen_code_buffer_write_to_file(CodeBuffer *code, char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return MG_ERR_CHECK_ERRNO;

    size_t nwritten = fwrite(code->buffer, sizeof(char), code->code_length, f);
    if (nwritten != code->code_length) {
#ifdef DEBUG
        fprintf(stderr, "only wrote %zu/%zu bytes to file `%s`",
                nwritten, code->code_length, filename);
#endif
        return MG_ERR_CHECK_ERRNO;
    }
    fclose(f);

    return MG_ERR_NONE;
}

void minigen_code_buffer_free(CodeBuffer *code) {
    if (code->buffer) {
        free(code->buffer);
        code->buffer = NULL;
    }
    code->code_length = code->code_capacity = 0;
}

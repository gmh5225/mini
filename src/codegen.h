#ifndef MCC_CODEGEN_H
#define MCC_CODEGEN_H

#include "ir.h"
#include "parse.h"
#include "table.h"
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_TARGET_CODE_CAPACITY 2048

typedef struct {
    char *buffer;
    size_t code_length;
    size_t code_capacity;
} CodeBuffer;

#define CB_WRITE(buf, bytes, len) \
    code_buffer_write_bytes(buf, bytes, len)

void code_buffer_init(CodeBuffer *code);
void code_buffer_write_bytes(CodeBuffer *code, char *bytes, size_t length);
void code_buffer_write_to_file(CodeBuffer *code, char *filename);
void code_buffer_free(CodeBuffer *code);

/* Available Backends */
CodeBuffer nasm_x86_64_generate(Program program);

#endif

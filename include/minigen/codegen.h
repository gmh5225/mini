#ifndef MINIGEN_CODEGEN_H
#define MINIGEN_CODEGEN_H

#include "minigen/ir.h"

#include <stddef.h>
#include <stdint.h>

typedef enum minigen_code_target {
    TARGET_LINUX_NASM_X86_64,
} CodeTarget;

#define DEFAULT_TARGET_CODE_CAPACITY 2048
typedef struct minigen_code_buffer CodeBuffer;

#define cb_write_bytes(buf, bytes, len) \
    minigen_code_buffer_write_bytes(buf, bytes, len)

void minigen_code_buffer_init(CodeBuffer *code, CodeTarget target);
void minigen_code_buffer_write_bytes(CodeBuffer *code, char *bytes, size_t length);
int minigen_code_buffer_write_to_file(CodeBuffer *code, char *filename);
void minigen_code_buffer_free(CodeBuffer *code);

#endif

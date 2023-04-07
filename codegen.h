#ifndef MINI_CODEGEN_H
#define MINI_CODEGEN_H

#include "ast.h"
#include "symbol_table.h"

#include <stddef.h>

typedef enum {
    TARGET_LINUX_NASM_X86_64,
} target_type;

extern const char *target_strings[];
const char *target_as_str(target_type type);

#define DEFAULT_TARGET_CODE_CAPACITY 2048

typedef struct {
    target_type type;
    char *generated_code;
    size_t code_length;
    size_t code_capacity;
} target_asm;

void target_asm_init(target_asm *out, target_type type);
void target_asm_generate_code(target_asm *out, ast_node *root, 
        symbol_table *symbols);
void target_asm_add_byte(target_asm *out, char byte);
void target_asm_add_bytes(target_asm *out, char *bytes, size_t length);
void target_asm_write_to_file(target_asm *out, char *output_filename);
void target_asm_free(target_asm *out);

#endif

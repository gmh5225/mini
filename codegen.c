#include "codegen.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Directives to allocate memory (in # bytes)
enum {
    DB = 1, DW = 2, DD = 4, DQ = 8,
    DO = 16, DY = 32, DZ = 64,
};

const char *mem_alloc_directive[] = {
    [DB] = "db", [DW] = "dw", [DD] = "dd", [DQ] = "dq",
    [DO] = "do", [DY] = "dy", [DZ] = "dz",
};

const char *target_strings[] = {
    [TARGET_LINUX_NASM_X86_64] = "linux_nasm_x86_64",
};

const char *target_as_str(target_type type) {
    return target_strings[type];
}

static void write_bytes(target_asm *out, const char *bytes, size_t length) {
    if (out->code_length + length >= out->code_capacity) {
        out->code_capacity <<= 1;
        void *tmp = realloc(out->generated_code, sizeof(char) * out->code_capacity);
        out->generated_code = tmp;
    }
#ifdef DEBUG
    printf("%.*s", (int)length, bytes);
#endif
    memcpy(out->generated_code + out->code_length, bytes, length);
    out->code_length += length;
}

void target_asm_init(target_asm *out, target_type type) {
    out->type = type;
    out->generated_code = calloc(DEFAULT_TARGET_CODE_CAPACITY, sizeof(char));
    out->code_length = 0;
    out->code_capacity = DEFAULT_TARGET_CODE_CAPACITY;
}

// Generate code using `root` and the `global_scope` table defined in symbol_table.c
void target_asm_generate_code(target_asm *out, ast_node *root) {
    // Initialize global variables
    // TODO: add support for global structures
    target_asm_add_section(out, ".data");
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        symbol_info *symbol = global_scope->symbols[i];
        if (symbol->name && symbol->type == SYMBOL_VARIABLE) {

            bool allocated = false;
            for (int sz = DB; sz <= DZ; sz <<= 1) {
                if (symbol->size == sz) {
                    const char *directive = mem_alloc_directive[symbol->size];
                    write_bytes(out, "\t", 1);
                    write_bytes(out, symbol->name, strlen(symbol->name));
                    write_bytes(out, " ", 1);
                    write_bytes(out, directive, strlen(directive));
                    allocated = true;
                }
            }

            if (!allocated) {
                fail("global struct initialization not yet supported!");
            }
        }
    }

    // Generate code for entry point of program
    target_asm_add_section(out, ".text");
    target_asm_add_instruction(out, "global", "_start", NULL);
    target_asm_add_label(out, "_start");
}

void target_asm_add_section(target_asm *out, char *section_name) {
    const char *section_text = "section ";
    write_bytes(out, section_text, strlen(section_text));
    write_bytes(out, section_name, strlen(section_name));
    write_bytes(out, "\n\n", 2);
}

void target_asm_add_label(target_asm *out, char *label_name) {
    write_bytes(out, label_name, strlen(label_name));
    write_bytes(out, ":\n", 2);
}

void target_asm_add_instruction(target_asm *out, char *inst, char *op1, char *op2) {
    write_bytes(out, "\t", 1);
    write_bytes(out, inst, strlen(inst));
    if (op1) { write_bytes(out, op1, strlen(op1)); }
    if (op2) { write_bytes(out, op2, strlen(op2)); }
    write_bytes(out, "\n", 1);
}

void target_asm_write_to_file(target_asm *out, char *output_filename) {
    FILE *f = fopen(output_filename, "w");
    if (!f) {
        fail("couldn't open output file '%s' for writing");
    }

    size_t nwritten = fwrite(out->generated_code, sizeof(char), out->code_length, f);
    if (nwritten != out->code_length) {
        fail("only wrote %zu/%zu bytes to file '%s'", nwritten, out->code_length, output_filename);
    }
    fclose(f);
}

void target_asm_free(target_asm *out) {
    if (out->generated_code) {
        free(out->generated_code);
    }
    out->code_length = out->code_capacity = 0;
}

#include "codegen.h"

#include <stdlib.h>

const char *target_strings[] = {
    [TARGET_LINUX_NASM_X86_64] = "linux_nasm_x86_64",
};

const char *target_as_str(target_type type) {
    return target_strings[type];
}

void target_asm_init(target_asm *out, target_type type) {
    out->type = type;
    out->generated_code = calloc(DEFAULT_TARGET_CODE_CAPACITY, sizeof(char));
    out->code_length = 0;
    out->code_capacity = DEFAULT_TARGET_CODE_CAPACITY;
}

void target_asm_generate_code(target_asm *out, ast_node *root, symbol_table *symbols) {

}

void target_asm_add_section(target_asm *out, char *section_name) {
    const char *section_text = "section ";
}

void target_asm_add_label(target_asm *out, char *label_name) {

}

void target_asm_add_instruction(target_asm *out, char *inst, char *op1, char *op2) {

}

void target_asm_write_to_file(target_asm *out, char *output_filename) {
}

void target_asm_free(target_asm *out) {
    if (out && out->generated_code) {
        free(out->generated_code);
    }
}

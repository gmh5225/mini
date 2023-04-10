#include "mini.h"

#define INDENT "    ", 4

// Directives to allocate memory (in # bytes)
enum {
    DB = 1, DW = 2, DD = 4, DQ = 8,
    DO = 16, DY = 32, DZ = 64,
};

const char *data_alloc_directive[] = {
    [DB] = "db", [DW] = "dw", [DD] = "dd", [DQ] = "dq",
    [DO] = "do", [DY] = "dy", [DZ] = "dz",
};

enum {
    RESB = 1, RESD = 4, RESQ = 8,
};

const char *bss_alloc_directive[] = {
    [RESB] = "resb", [RESD] = "resd", [RESQ] = "resq",
};

const char *target_strings[] = {
    [TARGET_LINUX_NASM_X86_64] = "linux_nasm_x86_64",
};

const char *target_as_str(TargetKind kind) {
    return target_strings[kind];
}

static void write_bytes(TargetASM *out, const char *bytes, size_t length) {
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

void target_asm_init(TargetASM *out, TargetKind kind) {
    out->kind = kind;
    out->generated_code = calloc(DEFAULT_TARGET_CODE_CAPACITY, sizeof(char));
    out->code_length = 0;
    out->code_capacity = DEFAULT_TARGET_CODE_CAPACITY;
}

// Generate code using `root` and the `global_scope` symbol table
void target_asm_generate_code(TargetASM *out, Node *root) {
    // Initialize global variables.
    // Since we do not have constant folding implemented yet,
    // we store all global variables in the bss section for now and compute their values
    // in the first few operations of the program.

    // TODO: Create two buffers, one for .data and one for .bss, and append to them
    // in one pass over the global symbol table. Once done, we can write them both
    // to the generated_code array.
    target_asm_add_section(out, ".bss");
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        Symbol *symbol = global_scope->symbols[i];
        if (symbol->name && symbol->kind == SYMBOL_VARIABLE) {
            Type type = symbol->type;
            write_bytes(out, INDENT);
            write_bytes(out, symbol->name, strlen(symbol->name));
            write_bytes(out, ": ", 2);

#define BUF_SZ 128
            // TODO: fix this
            char buf[BUF_SZ] = {0};
            int length = 0;

            // Try to reserve memory using the directive which is the GCD of the type size
            int rem;
            if ((rem = type.size % RESQ) == 0) {
                const char *directive = bss_alloc_directive[RESQ];
                write_bytes(out, directive, strlen(directive));
                length = snprintf(buf, BUF_SZ, " %d\n", rem);
                write_bytes(out, buf, length);
                continue;
            }

            if ((rem = type.size % RESD) == 0) {
                const char *directive = bss_alloc_directive[RESD];
                write_bytes(out, directive, strlen(directive));
                length = snprintf(buf, BUF_SZ, " %d\n", rem);
                write_bytes(out, buf, length);
                continue;
            }

            const char *directive = bss_alloc_directive[RESB];
            write_bytes(out, directive, strlen(directive));
            length = snprintf(buf, BUF_SZ, " %d\n", type.size);
            write_bytes(out, buf, length);
#undef BUF_SZ

            // For .data section:
            //if (symbol->is_initialized) {
            //    bool allocated = false;
            //    for (int sz = DB; sz <= DZ; sz <<= 1) {
            //        int size = symbol->type.size;
            //        if (size == sz) {
            //            const char *directive = mem_alloc_directive[size];
            //            write_bytes(out, INDENT);
            //            write_bytes(out, symbol->name, strlen(symbol->name));
            //            write_bytes(out, " ", 1);
            //            write_bytes(out, directive, strlen(directive));

            //            char literal[128];
            //            int length = 0;
            //            switch (symbol->type.kind) {
            //                case TYPE_VOID: 
            //                    error("cannot allocate memory for void type");
            //                    break;
            //                case TYPE_INT:
            //                    break;
            //                default: error("invalid type in codegen: %d", symbol->type.kind);
            //            }

            //            write_bytes(out, literal, length);
            //            allocated = true;
            //        }
            //    }

            //    if (!allocated) {
            //        error("global struct initialization not yet supported!");
            //    }
            //}
        }

    }

    // Generate code for entry point of program
    target_asm_add_section(out, ".text");
    target_asm_add_instruction(out, "global", "_start", NULL);
    target_asm_add_label(out, "_start");
}

void target_asm_add_section(TargetASM *out, char *section_name) {
    const char *section_text = "section ";
    write_bytes(out, section_text, strlen(section_text));
    write_bytes(out, section_name, strlen(section_name));
    write_bytes(out, "\n", 2);
}

void target_asm_add_label(TargetASM *out, char *label_name) {
    write_bytes(out, label_name, strlen(label_name));
    write_bytes(out, ":\n", 2);
}

void target_asm_add_instruction(TargetASM *out, char *inst, char *op1, char *op2) {
    write_bytes(out, INDENT);
    write_bytes(out, inst, strlen(inst));

    if (op1) { 
        write_bytes(out, " ", 1);
        write_bytes(out, op1, strlen(op1)); 
    }

    if (op2) { 
        write_bytes(out, ", ", 1);
        write_bytes(out, op2, strlen(op2));
    }

    write_bytes(out, "\n", 1);
}

void target_asm_write_to_file(TargetASM *out, char *output_filename) {
    FILE *f = fopen(output_filename, "w");
    if (!f) {
        error("couldn't open output file `%s` for writing");
    }

    size_t nwritten = fwrite(out->generated_code, sizeof(char), out->code_length, f);
    if (nwritten != out->code_length) {
        error("only wrote %zu/%zu bytes to file `%s`", nwritten, out->code_length, output_filename);
    }
    fclose(f);
}

void target_asm_free(TargetASM *out) {
    if (out->generated_code) {
        free(out->generated_code);
        out->generated_code = NULL;
    }
    out->code_length = out->code_capacity = 0;
}

#include "mini.h"

#define INDENT "    ", 4

// NASM x86_64 (Linux)

enum { R_RAX, R_RBX, R_RCX, R_RDX,
    R_RSP, R_RBP, R_RSI, R_RDI, 
    R_R8, R_R9, R_R10, R_R11, R_R12 };

static const Register registers_x86_64[] = {
    [R_RAX] = { "rax", false, true, NULL },
    [R_RBX] = { "rbx", true, false, NULL },
    [R_RCX] = { "rcx", false, true, NULL },
    [R_RDX] = { "rdx", false, true, NULL },
    [R_RSP] = { "rsp", true, false, NULL },
    [R_RBP] = { "rbp", true, false, NULL },
    [R_RSI] = { "rsi", false, true, NULL },
    [R_RDI] = { "rdi", false, true, NULL },
    [R_R8]  = { "r8", false, true, NULL },
    [R_R9]  = { "r9", false, true, NULL },
    [R_R10] = { "r10", false, true, NULL },
    [R_R11] = { "r11", false, true, NULL },
    [R_R12] = { "r12", true, false, NULL },
};

// Directives to allocate memory (in # bytes)
enum { DB = 1, DW = 2, DD = 4, DQ = 8,
    DO = 16, DY = 32, DZ = 64 };
const char *data_alloc_directive[] = {
    [DB] = "db", [DW] = "dw", [DD] = "dd", [DQ] = "dq",
    [DO] = "do", [DY] = "dy", [DZ] = "dz",
};

enum { RESB = 1, RESD = 4, RESQ = 8 };
const char *bss_alloc_directive[] = {
    [RESB] = "resb", [RESD] = "resd", [RESQ] = "resq",
};

const char *target_strings[] = {
    [TARGET_LINUX_NASM_X86_64] = "linux_nasm_x86_64",
};
const char *target_as_str(TargetKind kind) { return target_strings[kind]; }

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

static Register *next_register(TargetASM *out) {
    return NULL;
}

static void add_section(TargetASM *out, char *section_name) {
    const char *section_text = "section ";
    write_bytes(out, section_text, strlen(section_text));
    write_bytes(out, section_name, strlen(section_name));
    write_bytes(out, "\n", 2);
}

static void add_label(TargetASM *out, char *label_name) {
    write_bytes(out, label_name, strlen(label_name));
    write_bytes(out, ":\n", 2);
}

static void add_instruction(TargetASM *out, char *inst, char *op1, char *op2) {
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

static void generate_function(TargetASM *out, FuncDecl *func) {
    if (strcmp(func->name, "main") == 0) {
        add_label(out, "_start");
    } else {
        add_label(out, func->name);
    }
}

static void generate_variable(TargetASM *out, VarDecl *var) {
    if (symbol_table_lookup(global_scope, var->name)) return;
    error("stack local variable initialization not yet supported!");
}

static void generate(TargetASM *out, Node *root) {
    if (!root) return;

    switch (root->kind) {
        case NODE_FUNC_DECL:
            generate_function(out, &root->func_decl);
            break;
        case NODE_VAR_DECL:
            generate_variable(out, &root->var_decl);
            break;
        default:
            error("cannot generate code for node type: %d", root->kind);
    }

    generate(out, root->next);
}

void target_asm_init(TargetASM *out, TargetKind kind) {
    out->kind = kind;
    out->generated_code = calloc(DEFAULT_TARGET_CODE_CAPACITY, sizeof(char));
    out->code_length = 0;
    out->code_capacity = DEFAULT_TARGET_CODE_CAPACITY;

    // Create register stack to keep track of what registers we can use, 
    // placing the scratch registers at the top of the stack.
    Register scratch = {0};
    Register preserved = {0};
    Register *cur = &scratch;
    Register *save = &preserved;
    for (int id = R_RAX; id <= R_R12; id++) {
        Register *reg = malloc(sizeof(Register));
        *reg = registers_x86_64[id];

        if (reg->is_preserved) {
            save = save->next = reg;
        } else {
            cur = cur->next = reg;
        }
    }
    cur = cur->next = preserved.next;
    out->registers = scratch.next;

#ifdef DEBUG
    printf("Open Registers:\n");
    Register *iter = out->registers;
    while (iter) {
        printf("%*s%s\t%s\n", 4, "",
                iter->name, iter->is_preserved ? "preserved" : "scratch");
        iter = iter->next;
    }
#endif
}

// Generate code using `root` and the `global_scope` symbol table
void target_asm_generate_code(TargetASM *out, Node *program) {
    // Initialize global variables.
    // Since we do not have constant folding implemented yet,
    // we store all global variables in the bss section for now and compute their values
    // in the first few operations of the program.

    // TODO: Create two buffers, one for .data and one for .bss, and append to them
    // in one pass over the global symbol table. Once done, we can write them both
    // to the generated_code array.
    add_section(out, ".bss");
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
    add_section(out, ".text");
    add_instruction(out, "global", "_start", NULL);

    // Generate the program recursively
    generate(out, program);
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

    Register *iter = out->registers;
    while (iter) {
        Register *temp = iter->next;
        free(iter);
        iter = temp;
    }
}

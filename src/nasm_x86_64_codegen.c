#include "minigen/codegen.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NASM x86_64 (Linux)

typedef struct {
    int id;
    char *name;
    bool is_preserved;
    bool is_active;
} Register;

enum { R_RAX, R_RBX, R_RCX, R_RDX,
    R_RSP, R_RBP, R_RSI, R_RDI, 
    R_R8, R_R9, R_R10, R_R11, R_R12 };

#define NUM_REGISTERS R_R12 + 1

static const Register registers_nasm_x86_64[] = {
    [R_RAX] = { R_RAX, "rax", false, false },
    [R_RBX] = { R_RBX, "rbx", true, false },
    [R_RCX] = { R_RCX, "rcx", false, false },
    [R_RDX] = { R_RDX, "rdx", false, false },
    [R_RSP] = { R_RSP, "rsp", true, false },
    [R_RBP] = { R_RBP, "rbp", true, false },
    [R_RSI] = { R_RSI, "rsi", false, false },
    [R_RDI] = { R_RDI, "rdi", false, false },
    [R_R8]  = { R_R8, "r8", false, false },
    [R_R9]  = { R_R9, "r9", false, false },
    [R_R10] = { R_R10, "r10", false, false },
    [R_R11] = { R_R11, "r11", false, false },
    [R_R12] = { R_R12, "r12", true, false },
};

typedef struct {
    Register registers[NUM_REGISTERS];
    CodeBuffer *code;
} CodegenState;

static void codegen_state_init(CodegenState *state, CodeBuffer *code) {
    state->code = code;
#ifdef DEBUG
    printf("Available Registers:\n");
#endif
    for (int id = R_RAX; id <= R_R12; id++) {
        state->registers[id] = registers_nasm_x86_64[id];
#ifdef DEBUG
        printf("%*s%s\t%s\t%s\n", 4, "",
                state->registers[id].name,
                state->registers[id].is_preserved ? "preserved" : "scratch",
                state->registers[id].is_active ? "active" : "");
#endif
    }
}

/* Simple Linear Scan Register Allocator */
static void save_register_to_stack(CodegenState *state, int id) {
    Register *reg = &state->registers[id];
}

static Register *find_available_register(CodegenState *state) {
    for (int id = R_RAX; id <= R_R12; id++) {
        Register *reg = &state->registers[id];
        if (!reg->is_active) {
            reg->is_active = true;

            return reg;
        }
    }
    return NULL;
}

static void release_register(CodegenState *state, int id) {
    Register *reg = &state->registers[id];
    reg->is_active = false;
}

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

static void add_section(CodegenState *state, char *section_name) {
    CodeBuffer *buf = state->code;
    cb_write_bytes(buf, "section ", 8);
    cb_write_bytes(buf, section_name, strlen(section_name));
    cb_write_bytes(buf, "\n", 2);
}

static void add_label(CodegenState *state, char *label_name) {
    CodeBuffer *buf = state->code;
    cb_write_bytes(buf, label_name, strlen(label_name));
    cb_write_bytes(buf, ":\n", 2);
}

static void add_instruction(CodegenState *state, char *inst, char *op1, char *op2) {
    CodeBuffer *buf = state->code;
    cb_write_bytes(buf, "    ", 4);
    cb_write_bytes(buf, inst, strlen(inst));

    if (op1) { 
        cb_write_bytes(buf, " ", 1);
        cb_write_bytes(buf, op1, strlen(op1)); 
    }

    if (op2) { 
        cb_write_bytes(buf, ", ", 1);
        cb_write_bytes(buf, op2, strlen(op2));
    }

    cb_write_bytes(buf, "\n", 1);
}

void nasm_x86_64_generate(MinigenIR *ir, CodeBuffer *code) {
    CodegenState state = {0};
    codegen_state_init(&state, code);

    // Initialize global variables.
    // Since we do not have constant folding implemented yet,
    // we store all global variables in the bss section for now and compute their values
    // in the first few operations of the program.

    // TODO: Create two buffers, one for .data and one for .bss, and append to them
    // in one pass over the global symbol table. Once done, we can write them both
    // to the generated_code array.
    add_section(&state, ".bss");

    add_section(&state, ".text");
    add_instruction(&state, "global", "_start", NULL);

    /*
       for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
       symbol *symbol = global_scope->symbols[i];
       if (symbol->name && symbol->kind == SYMBOL_VARIABLE) {
       type type = symbol->type;
       cb_write_bytes(code->buffer, "    ", 4);
       cb_write_bytes(code->buffer, symbol->name, strlen(symbol->name));
       cb_write_bytes(code->buffer, ": ", 2);

#define BUF_SZ 128
    // TODO: fix this
    char buf[BUF_SZ] = {0};
    int length = 0;

    // Try to reserve memory using the directive which is the GCD of the type size
    int rem;
    if ((rem = type.size % RESQ) == 0) {
    const char *directive = bss_alloc_directive[RESQ];
    cb_write_bytes(code->buffer, directive, strlen(directive));
    length = snprintf(buf, BUF_SZ, " %d\n", rem);
    cb_write_bytes(code->buffer, buf, length);
    continue;
    }

    if ((rem = type.size % RESD) == 0) {
    const char *directive = bss_alloc_directive[RESD];
    cb_write_bytes(code->buffer, directive, strlen(directive));
    length = snprintf(buf, BUF_SZ, " %d\n", rem);
    cb_write_bytes(code->buffer, buf, length);
    continue;
    }

    const char *directive = bss_alloc_directive[RESB];
    cb_write_bytes(code->buffer, directive, strlen(directive));
    length = snprintf(buf, BUF_SZ, " %d\n", type.size);
    cb_write_bytes(code->buffer, buf, length);
#undef BUF_SZ

    // For .data section:
    //if (symbol->is_initialized) {
    //    bool allocated = false;
    //    for (int sz = DB; sz <= DZ; sz <<= 1) {
    //        int size = symbol->type.size;
    //        if (size == sz) {
    //            const char *directive = mem_alloc_directive[size];
    //            cb_write_bytes(code->buffer, INDENT);
    //            cb_write_bytes(code->buffer, symbol->name, strlen(symbol->name));
    //            cb_write_bytes(code->buffer, " ", 1);
    //            cb_write_bytes(code->buffer, directive, strlen(directive));

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

    //            cb_write_bytes(code->buffer, literal, length);
    //            allocated = true;
    //        }
    //    }

    //    if (!allocated) {
    //        error("global struct initialization not yet supported!");
    //    }
    //}
    }
}
*/
}
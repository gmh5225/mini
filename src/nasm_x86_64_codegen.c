#include "symbols.h"
#include "types.h"
#include "codegen.h"
#include "compile.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NASM x86_64 (Linux)

typedef enum RegisterID RegisterID;
typedef struct Register Register;
typedef struct CodeGenerator CodeGenerator;

enum RegisterID
{
    R_RAX,
    R_RBX, 
    R_RCX,
    R_RDX,
    R_RSP, 
    R_RBP, 
    R_RSI, 
    R_RDI,
    R_R8,
    R_R9,
    R_R10, 
    R_R11, 
    R_R12,
};

struct Register
{
    RegisterID id;
    char *name;
    bool is_preserved;
    bool is_active;
};


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
    [R_R8]  = { R_R8,  "r8", false, false },
    [R_R9]  = { R_R9,  "r9", false, false },
    [R_R10] = { R_R10, "r10", false, false },
    [R_R11] = { R_R11, "r11", false, false },
    [R_R12] = { R_R12, "r12", true, false },
};

struct CodeGenerator
{
    Register registers[NUM_REGISTERS];
    CodeBuffer code;
};

static void code_generator_init(CodeGenerator *gen)
{
    code_buffer_init(&gen->code);
#ifdef DEBUG
    printf("Available Registers:\n");
#endif
    for (RegisterID id = R_RAX; id <= R_R12; id++) {
        gen->registers[id] = registers_nasm_x86_64[id];
#ifdef DEBUG
        printf("%*s%s\t%s\t%s\n", 4, "",
                gen->registers[id].name,
                gen->registers[id].is_preserved ? "preserved" : "scratch",
                gen->registers[id].is_active ? "active" : "");
#endif
    }
}

/* Simple Linear Scan Register Allocator */
static void save_register_to_stack(CodeGenerator *gen, int id)
{
    Register *reg = &gen->registers[id];
}

static Register *find_available_register(CodeGenerator *gen)
{
    for (RegisterID id = R_RAX; id <= R_R12; id++) {
        Register *reg = &gen->registers[id];
        if (!reg->is_active) {
            reg->is_active = true;
            return reg;
        }
    }
    return NULL;
}

static void release_register(CodeGenerator *gen, int id)
{
    Register *reg = &gen->registers[id];
    reg->is_active = false;
}

// Directives to allocate memory (in # bytes)
enum
{ 
    DB = 1,
    DW = 2,
    DD = 4,
    DQ = 8,
    DO = 16,
    DY = 32,
    DZ = 64,
};

const char *data_alloc_directive[] = {
    [DB] = "db",
    [DW] = "dw",
    [DD] = "dd",
    [DQ] = "dq",
    [DO] = "do",
    [DY] = "dy",
    [DZ] = "dz",
};

enum
{
    RESB = 1,
    RESD = 4,
    RESQ = 8,
};

const char *bss_alloc_directive[] = {
    [RESB] = "resb",
    [RESD] = "resd",
    [RESQ] = "resq",
};

static void add_section(CodeGenerator *gen, char *section_name)
{
    CodeBuffer *buf = &gen->code;
    CB_WRITE(buf, "section ", 8);
    CB_WRITE(buf, section_name, strlen(section_name));
    CB_WRITE(buf, "\n", 2);
}

static void add_label(CodeGenerator *gen, char *label_name) {
    CodeBuffer *buf = &gen->code;
    CB_WRITE(buf, label_name, strlen(label_name));
    CB_WRITE(buf, ":\n", 2);
}

static void add_instruction(CodeGenerator *gen, char *inst, char *op1, char *op2) {
    CodeBuffer *buf = &gen->code;
    CB_WRITE(buf, "    ", 4);
    CB_WRITE(buf, inst, strlen(inst));

    if (op1) { 
        CB_WRITE(buf, " ", 1);
        CB_WRITE(buf, op1, strlen(op1)); 
    }

    if (op2) { 
        CB_WRITE(buf, ", ", 1);
        CB_WRITE(buf, op2, strlen(op2));
    }

    CB_WRITE(buf, "\n", 1);
}

#define DATA_SZ 32

CodeBuffer nasm_x86_64_generate(Program program)
{
    CodeGenerator gen = {0};
    code_generator_init(&gen);
    CodeBuffer *buf = &gen.code;

    // Allocate space for uninitialized global variables
    add_section(&gen, ".bss");

    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        Symbol *symbol = global_scope->symbols[i];
        if (symbol->name && symbol->kind == SYMBOL_VARIABLE) {
            Type type = symbol->type;
            CB_WRITE(buf, "    ", 4);
            CB_WRITE(buf, symbol->name, strlen(symbol->name));
            CB_WRITE(buf, ": ", 2);

            // TODO: fix this
            char data[DATA_SZ] = {0};
            int length = 0;

            // Try to reserve memory using the directive which is the GCD of the type size
            int rem;
            if ((rem = type.size % RESQ) == 0) {
                const char *directive = bss_alloc_directive[RESQ];
                CB_WRITE(buf, directive, strlen(directive));
                length = snprintf(data, DATA_SZ, " %d\n", rem);
                CB_WRITE(buf, data, length);
                continue;
            }

            if ((rem = type.size % RESD) == 0) {
                const char *directive = bss_alloc_directive[RESD];
                CB_WRITE(buf, directive, strlen(directive));
                length = snprintf(data, DATA_SZ, " %d\n", rem);
                CB_WRITE(buf, data, length);
                continue;
            }

            const char *directive = bss_alloc_directive[RESB];
            CB_WRITE(buf, directive, strlen(directive));
            length = snprintf(data, DATA_SZ, " %d\n", type.size);
            CB_WRITE(buf, data, length);

            // For .data section:
            //if (symbol->is_initialized) {
            //    bool allocated = false;
            //    for (int sz = DB; sz <= DZ; sz <<= 1) {
            //        int size = symbol->type.size;
            //        if (size == sz) {
            //            const char *directive = mem_alloc_directive[size];
            //            CB_WRITE(code->buffer, INDENT);
            //            CB_WRITE(code->buffer, symbol->name, strlen(symbol->name));
            //            CB_WRITE(code->buffer, " ", 1);
            //            CB_WRITE(code->buffer, directive, strlen(directive));

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

            //            CB_WRITE(code->buffer, literal, length);
            //            allocated = true;
            //        }
            //    }

            //    if (!allocated) {
            //        error("global struct initialization not yet supported!");
            //    }
            //}
        }
    }

    add_section(&gen, ".text");
    add_instruction(&gen, "global", "_start", NULL);

    return gen.code;
}

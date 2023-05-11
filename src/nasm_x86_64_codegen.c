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
  NUM_REGISTERS,
};

struct Register
{
  RegisterID id;
  char *name;
  bool is_preserved;
  bool is_active;
  int start, end;
};

static const Register REGISTERS_NASM_x86_64[] = {
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

/* Codegen State */
static Register registers[NUM_REGISTERS];

/* Simple Linear Scan Register Allocator */
static void spill_register_to_stack(RegisterID id)
{
  Register *reg = &registers[id];
}

static Register *find_available_register()
{
  for (RegisterID id = R_RAX; id <= R_R12; id++) {
    Register *reg = &registers[id];
    if (!reg->is_active) {
      reg->is_active = true;
      return reg;
    }
  }
  return NULL;
}

static void release_register(RegisterID id)
{
  Register *reg = &registers[id];
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

const char *init_mem[] = {
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

const char *uninit_mem[] = {
  [RESB] = "resb",
  [RESD] = "resd",
  [RESQ] = "resq",
};

static void add_bytes(char *bytes, size_t length)
{

}

static void add_section(char *section)
{
  add_bytes("section ", 8);
  add_bytes(section, strlen(section));
  add_bytes("\n", 2);
}

static void add_label(char *label) {
  add_bytes(label, strlen(label));
  add_bytes(":\n", 2);
}

static void add_instruction(char *instruction, char *op1, char *op2) {
  add_bytes("    ", 4);
  add_bytes(instruction, strlen(instruction));

  if (op1) { 
    add_bytes(" ", 1);
    add_bytes(op1, strlen(op1)); 
  }

  if (op2) { 
    add_bytes(", ", 1);
    add_bytes(op2, strlen(op2));
  }

  add_bytes("\n", 1);
}

int nasm_x86_64_generate(ControlFlowGraph *graph)
{
#ifdef DEBUG
  printf("Available Registers:\n");
#endif
  for (RegisterID id = R_RAX; id <= R_R12; id++) {
    registers[id] = REGISTERS_NASM_x86_64[id];
#ifdef DEBUG
    printf("%*s%s\t%s\t%s\n", 4, "",
        registers[id].name,
        registers[id].is_preserved ? "preserved" : "scratch",
        registers[id].is_active ? "active" : "");
#endif
  }

  // Allocate space for uninitialized global variables
  add_section(".bss");

  for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
    Symbol *symbol = ctx->global_scope->symbols[i];
    if (symbol->name && symbol->kind == SYMBOL_VARIABLE) {
      Type type = symbol->type;
      add_bytes("    ", 4);
      add_bytes(symbol->name, strlen(symbol->name));
      add_bytes(": ", 2);

#define DATA_SZ 32
      // TODO: fix this
      char data[DATA_SZ] = { 0 };
      int length = 0;

      // Try to reserve memory using the directive which is the GCD of the type size
      int rem = 0;
      uint8_t directives[2] = { RESQ, RESD };
      for (uint8_t d = 0; d < 2; d++) {
        if ((rem = type.size % directives[d]) == 0) {
          add_bytes(uninit_mem[d], 4);
          length = snprintf(data, DATA_SZ, " %d\n", rem);
          add_bytes(data, length);
        }
      }

      if (length == 0) {
        add_bytes(uninit_mem[RESB], 4);
        length = snprintf(data, DATA_SZ, " %d\n", rem);
        add_bytes(data, length);
      }

      // For .data section:
      //if (symbol->is_initialized) {
      //    bool allocated = false;
      //    for (int sz = DB; sz <= DZ; sz <<= 1) {
      //        int size = symbol->type.size;
      //        if (size == sz) {
      //            const char *directive = mem_alloc_directive[size];
      //            add_bytes(code->buffer, INDENT);
      //            add_bytes(code->buffer, symbol->name, strlen(symbol->name));
      //            add_bytes(code->buffer, " ", 1);
      //            add_bytes(code->buffer, directive, strlen(directive));

      //            char literal[128];
      //            int length = 0;
      //            switch (symbol->type.kind) {
      //                case TYPE_VOID: 
      //                    fatal("cannot allocate memory for void type");
      //                    break;
      //                case TYPE_INT:
      //                    break;
      //                default: fatal("invalid type in codegen: %d", symbol->type.kind);
      //            }

      //            add_bytes(code->buffer, literal, length);
      //            allocated = true;
      //        }
      //    }

      //    if (!allocated) {
      //        fatal("global struct initialization not yet supported!");
      //    }
      //}
    }
  }

  add_section(".text");
  add_instruction("global", "_start", NULL);

  add_label("_start");

  return 0;
}

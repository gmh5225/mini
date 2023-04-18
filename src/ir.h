#ifndef MINI_IR_H
#define MINI_IR_H

#include "parse.h"
#include "table.h"
#include "types.h"
#include "vector.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* MINI Intermediate Representation */

typedef enum OpCode OpCode;
typedef enum OperandKind OperandKind;
typedef struct Operand Operand;
typedef struct Instruction Instruction;
typedef struct BasicBlock BasicBlock;
typedef struct Program Program;

struct BasicBlock
{
    int id;
    char *tag;
    Vector predecessors;    // BasicBlock *
    Vector successors;      // BasicBlock *
    Vector instructions;    // Instruction *
    Table  *variables;      // char *
    BasicBlock *next;
};

struct Program
{
    BasicBlock *entry;
    BasicBlock *exit;
    BasicBlock *blocks;
    int num_blocks;
};

// NOTE: The order of these matter
enum OpCode
{
    OP_UNKNOWN     = 0,
    OP_ADD         = BIN_ADD,
    OP_SUB         = BIN_SUB,
    OP_MUL         = BIN_MUL,
    OP_DIV         = BIN_DIV,
    OP_NEG         = UN_NEG,
    OP_NOT         = UN_NOT,
    OP_CMP         = BIN_CMP,
    OP_CMP_NOT     = BIN_CMP_NOT,
    OP_CMP_LT      = BIN_CMP_LT,
    OP_CMP_GT      = BIN_CMP_GT,
    OP_CMP_LT_EQ   = BIN_CMP_LT_EQ,
    OP_CMP_GT_EQ   = BIN_CMP_GT_EQ,
    OP_DEF,
    OP_LOAD,
    OP_STORE,
    OP_JMP,
    OP_BR,
    OP_RET,
};

enum OperandKind
{
    OPERAND_UNKNOWN,
    OPERAND_LITERAL,
    OPERAND_TYPE,
    OPERAND_REGISTER,
    OPERAND_LABEL,
};

struct Operand
{
    OperandKind kind;
    union
    {
        Literal literal;
        Type type;
        char *reg_name;
        char *label_name;
    };
};

#define MAX_OPERANDS 4

struct Instruction
{
    OpCode opcode;
    Operand operands[MAX_OPERANDS];
    size_t num_operands;
};

Program emit_ir(ASTNode *root);
void dump_instruction(Instruction *inst);

#endif

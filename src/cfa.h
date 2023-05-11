#ifndef MINI_CFA_H
#define MINI_CFA_H

#include "parse.h"
#include "table.h"
#include "types.h"
#include "vector.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum OpCode OpCode;
typedef enum OperandKind OperandKind;
typedef struct Operand Operand;
typedef struct Instruction Instruction;
typedef struct BasicBlock BasicBlock;
typedef struct ControlFlowGraph ControlFlowGraph;

/* MINI Intermediate Representation (Three-Address Code) */

// NOTE: The order of these matter
enum OpCode
{
  OP_UNKNOWN     = 0,
  OP_NEG         = UN_NEG,
  OP_NOT         = UN_NOT,
  OP_DEREF       = UN_DEREF,
  OP_ADDR        = UN_ADDR,
  OP_ADD         = BIN_ADD,
  OP_SUB         = BIN_SUB,
  OP_MUL         = BIN_MUL,
  OP_DIV         = BIN_DIV,
  OP_CMP         = BIN_CMP,
  OP_CMP_NOT     = BIN_CMP_NOT,
  OP_CMP_LT      = BIN_CMP_LT,
  OP_CMP_GT      = BIN_CMP_GT,
  OP_CMP_LT_EQ   = BIN_CMP_LT_EQ,
  OP_CMP_GT_EQ   = BIN_CMP_GT_EQ,
  OP_DEF,
  OP_ASSIGN,
  OP_JMP,
  OP_BR,
  OP_RET,
};

enum OperandKind
{
  OPERAND_UNKNOWN,
  OPERAND_LITERAL,
  OPERAND_VARIABLE,
  OPERAND_LABEL,
};

struct Operand
{
  OperandKind kind;
  Type type;
  union
  {
    Value literal;
    char *var;
    char *label;
  };
};

#define MAX_OPERANDS 2

struct Instruction
{
  OpCode opcode;
  char *assignee;
  Operand operands[MAX_OPERANDS];
  uint8_t num_operands;
};

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

struct ControlFlowGraph
{
  BasicBlock *entry;
  BasicBlock *exit;
  BasicBlock *blocks;
  int num_blocks;
};

ControlFlowGraph construct_cfg(Node *root);
void dump_instruction(Instruction *inst);

#endif

#include "cfa.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Table *var_names = NULL;
static Table *expressions = NULL;
static BasicBlock *blocks = NULL;
static BasicBlock *current_block = NULL;
static int block_count = 0;
static int num_temporaries = 0;

static Node *emit(Node *);
static void add_operand(Instruction *, void *, OperandKind);

static BasicBlock *make_basic_block(char *tag, int id)
{
  BasicBlock *block = calloc(1, sizeof(BasicBlock));
  block->id = id;
  block->tag = tag;
  vector_init(&block->predecessors, sizeof(BasicBlock *));
  vector_init(&block->successors, sizeof(BasicBlock *));
  vector_init(&block->instructions, sizeof(Instruction *));
  block->variables = table_new();
  return block;
}

static Instruction *make_instruction(OpCode opcode)
{
  Instruction *instruction = calloc(1, sizeof(Instruction));
  instruction->opcode = opcode;
  return instruction;
}

static BasicBlock *add_block(char *tag)
{
  BasicBlock *block = make_basic_block(tag, block_count++);

  // First block
  if (!current_block) {
    current_block = block;
    blocks = current_block;
  }
  else {
    // Add BasicBlocks to respective ancestor lists
    vector_push_back(&block->predecessors, current_block);
    vector_push_back(&current_block->successors, block);

    current_block->next = block;
    current_block = current_block->next;
  }
  return current_block;
}

static char *encode_instruction(Instruction *inst)
{
  size_t size = 1 + (sizeof(Operand) * inst->num_operands);
  uint8_t *buf = calloc(size, sizeof(uint8_t));
  memcpy(buf, &inst->opcode, 1);

  for (uint8_t i = 0; i < inst->num_operands; i++)
    memcpy(buf, &inst->operands[i], sizeof(Operand));

  uint64_t inst_hash = hash_n(buf, size);
  char *encoded = aprintf("%zu", inst_hash);

  /*
#ifdef DEBUG
  printf("encoded %s = ", inst->assignee);
  for (size_t i = 0; i < strlen(encoded); i++) {
    printf("%X ", encoded[i]);
  }
  printf("\n");
#endif
  */

  return encoded;
}

static void add_instruction(Instruction *inst)
{
  if (!current_block)
    fatal("no block to add instruction to");

  if (inst->assignee) {
    char *encoded = encode_instruction(inst);
    char *exists = (char *)table_lookup(expressions, encoded);
    if (exists) {
      LOG_INFO("eliminating redundant calculation for variable `%s`", inst->assignee);
      inst->opcode = OP_ASSIGN;
      memset(inst->operands, 0, sizeof(Operand) * MAX_OPERANDS);
      inst->num_operands = 0;
      add_operand(inst, exists, OPERAND_VARIABLE);
    } else {
      table_insert(expressions, encoded, inst->assignee);
    }
  }

  vector_push_back(&current_block->instructions, inst);
}

static Instruction *previous_instruction()
{
  if (!current_block)
    fatal("no block to get previous instruction from");

  Instruction *inst = (Instruction *)vector_get(
      &current_block->instructions,
      current_block->instructions.size - 1
      );
  return inst;
}

static void add_operand(Instruction *inst, void *value, OperandKind kind)
{
  if (inst->num_operands == MAX_OPERANDS)
    fatal("too many operands for instruction %d", inst->opcode);

  Operand operand = { .kind = kind };
  switch (operand.kind) {
    case OPERAND_LITERAL:
      operand.literal = *((Value *)value);
      break;
    case OPERAND_VARIABLE:
      operand.var = (char *)value;
      break;
    case OPERAND_LABEL:
      operand.label = (char *)value;
      break;
    default: fatal("invalid OperandKind: %d", kind);
  }

  inst->operands[inst->num_operands++] = operand;
}

static void add_operands_from_node(Instruction *inst, Node *node)
{
  switch (node->kind) {
    case NODE_LITERAL_EXPR:
      add_operand(inst, &node->literal, OPERAND_LITERAL);
      break;
    case NODE_REF_EXPR:
      add_operand(inst, node->ref, OPERAND_VARIABLE);
      break;
    default:
      emit(node);
      Instruction *temporary = previous_instruction();
      add_operand(inst, temporary->assignee, OPERAND_VARIABLE);
  }
}

char *create_temporary()
{
  int temporary_id = num_temporaries++;
  char *temporary_name = aprintf("$t%d", temporary_id);
  return temporary_name;
}

static Node *emit(Node *node)
{
  if (!node) return NULL;
  node->visited = true;
  Node *next = node->next;
  Instruction *inst = NULL;

  switch (node->kind) {
    case NODE_NOOP:
      free(node);
      break;
    case NODE_FUNC_DECL:
      add_block(node->func_decl.name);
      inst = make_instruction(OP_DEF);
      add_operand(inst, node->func_decl.name, OPERAND_LABEL);
      add_instruction(inst);
      emit(node->func_decl.params);
      emit(node->func_decl.body);
      break;
    case NODE_VAR_DECL:
      inst = make_instruction(OP_ASSIGN);
      add_operands_from_node(inst, node->var_decl.init);
      inst->assignee = node->var_decl.name;
      table_insert(var_names, inst->assignee, inst->assignee);
      add_instruction(inst);
      break;
    case NODE_ASSIGN_STMT:
      inst = make_instruction(OP_ASSIGN);
      add_operands_from_node(inst, node->assign.value);
      inst->assignee = node->assign.name;
      add_instruction(inst);
      break;
    case NODE_COND_STMT:
      fatal("conditional translation to IR is not implemented yet");
      break;
    case NODE_RET_STMT:
      inst = make_instruction(OP_RET);
      add_operands_from_node(inst, node->ret_stmt.value);
      add_instruction(inst);
      break;
    case NODE_UNARY_EXPR:
      inst = make_instruction((OpCode)node->unary.un_op);
      add_operands_from_node(inst, node->unary.expr);
      inst->assignee = create_temporary();
      add_instruction(inst);
      break;
    case NODE_BINARY_EXPR:
      inst = make_instruction((OpCode)node->binary.bin_op);
      Node *exprs[2] = { node->binary.lhs, node->binary.rhs };
      for (uint8_t i = 0; i < 2; i++) {
        Node *expr = exprs[i];
        add_operands_from_node(inst, expr);
      }
      inst->assignee = create_temporary();
      add_instruction(inst);
      break;
    case NODE_LITERAL_EXPR: // Leaf node
    case NODE_REF_EXPR:
      return node;
    default: fatal("cannot emit IR from node: %d", node->kind);
  }

  emit(next);
  return NULL;
}

ControlFlowGraph construct_cfg(Node *root)
{
  var_names = table_new();
  expressions = table_new();
  blocks = current_block = NULL;
  block_count = 0;
  num_temporaries = 0;

  ControlFlowGraph graph = { 0 };
  graph.entry = add_block("$entry");

  emit(root);

  graph.exit = add_block("$exit");
  graph.blocks = blocks;
  graph.num_blocks = block_count;

  table_free(var_names);
  table_free(expressions);

  return graph;
}

void dump_operand(Operand operand)
{
  switch (operand.kind) {
    case OPERAND_LITERAL:
      dump_value(operand.literal);
      break;
    case OPERAND_VARIABLE:
      printf("%s", operand.var);
      break;
    case OPERAND_LABEL:
      printf("%s", operand.label);
      break;
    default: fatal("invalid OperandKind: %d", operand.kind);
  }
}

char *opcode_as_str(OpCode opcode)
{
  switch (opcode) {
    case OP_ADD:
      return "+";
    case OP_NEG:
    case OP_SUB: return "-";
    case OP_MUL: return "*";
    case OP_DIV: return "/";
    case OP_NOT: return "!";
    case OP_CMP: return "==";
    case OP_CMP_NOT: return "!=";
    case OP_CMP_LT: return "<";
    case OP_CMP_GT: return ">";
    case OP_CMP_LT_EQ: return "<=";
    case OP_CMP_GT_EQ: return ">=";
    default: fatal("invalid OpCode %d", opcode);
  }
  return "";
}

void dump_instruction(Instruction *inst)
{
  switch (inst->opcode) {
    case OP_DEF:
      assert(inst->num_operands == 1);
      printf("def ");
      dump_operand(inst->operands[0]);
      break;
    case OP_ASSIGN:
      assert(inst->num_operands == 1);
      printf("  %s := ", inst->assignee);
      dump_operand(inst->operands[0]);
      break;
    case OP_NEG:
    case OP_NOT:
      assert(inst->num_operands == 1);
      printf("  %s := ", inst->assignee);
      printf(opcode_as_str(inst->opcode));
      dump_operand(inst->operands[0]);
      break;
    case OP_ADD: // Binary Ops
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_CMP:
    case OP_CMP_NOT:
    case OP_CMP_LT:
    case OP_CMP_GT:
    case OP_CMP_LT_EQ:
    case OP_CMP_GT_EQ:
      assert(inst->num_operands == 2);
      printf("  %s := ", inst->assignee);
      dump_operand(inst->operands[0]);
      printf(opcode_as_str(inst->opcode));
      dump_operand(inst->operands[1]);
      break;
    case OP_RET:
      assert(inst->num_operands == 1);
      printf("  ret ");
      dump_operand(inst->operands[0]);
      break;
    default: fatal("invalid Instruction: %d", inst->opcode);
  }
  printf("\n");
}

#include "ir.h"
#include "util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct IRBuilder IRBuilder;
struct IRBuilder
{
    Table *var_names;
    Table *expressions;
    BasicBlock *blocks;
    BasicBlock *current_block;
    int block_count;
};

static ASTNode *emit(IRBuilder *, ASTNode *);
static void add_operand(Instruction *, void *, OperandKind);

/* IRBuilder */
static void ir_builder_init(IRBuilder *builder)
{
    builder->var_names = table_new();
    builder->expressions = table_new();
    builder->blocks = builder->current_block = NULL;
    builder->block_count = 0;
}

static void ir_builder_free(IRBuilder *builder)
{
    table_free(builder->var_names);
}

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

static void add_block(IRBuilder *builder, char *tag)
{
    BasicBlock *block = make_basic_block(tag, builder->block_count++);

    // First block
    if (!builder->current_block) {
        builder->current_block = block;
        builder->blocks = builder->current_block;
        return;
    }

    // Add BasicBlocks to respective ancestor lists
    vector_push_back(&block->predecessors, builder->current_block);
    vector_push_back(&builder->current_block->successors, block);

    builder->current_block->next = block;
    builder->current_block = builder->current_block->next;
}

static char *encode_instruction(IRBuilder *builder, Instruction *inst)
{
    size_t capacity = 16;
    char *encoded = calloc(capacity, sizeof(char));
    size_t size = 0;

    for (size_t i = 0; i < inst->num_operands; i++) {
        Operand operand = inst->operands[i];
        switch (operand.kind) {
            case OPERAND_LITERAL:
                size_t literal_bytes_size = 0;
                char *literal_bytes = copy_literal_bytes(&operand.literal, &literal_bytes_size);
                if (size + literal_bytes_size >= capacity) {
                    capacity += literal_bytes_size + 1;
                    void *tmp = realloc(encoded, capacity * sizeof(char));
                    encoded = tmp;
                }
                memcpy(encoded + size, literal_bytes, literal_bytes_size);
                size += literal_bytes_size;
                break;
            case OPERAND_VARIABLE:
                size_t var_name_size = strlen(operand.var);
                if (size + var_name_size >= capacity) {
                    capacity += var_name_size + 1;
                    void *tmp = realloc(encoded, capacity * sizeof(char));
                    encoded = tmp;
                }
                memcpy(encoded + size, operand.var, var_name_size);
                size += var_name_size;
                break;
            default: error("cannot encode invalid OperandKind: %d", operand.kind);
        }
    }
    encoded[size] = 0;

#ifdef DEBUG
    printf("Encoded TAC: ");
    for (size_t i = 0; i < size; i++) {
        printf("%X ", encoded[i]);
    }
    printf("\n");
#endif

    return encoded;
}

static void add_instruction(IRBuilder *builder, Instruction *inst)
{
    BasicBlock *current_block = builder->current_block;
    if (!current_block)
        error("no block to add instruction to");

    if (inst->assignee) {
        char *encoded = encode_instruction(builder, inst);
        char *exists = (char *)table_lookup(builder->expressions, encoded);
        if (exists) {
            LOG_INFO("eliminating redundant calculation for variable `%s`",
                    inst->assignee);
            inst->opcode = OP_ASSIGN;
            memset(inst->operands, 0, sizeof(Operand) * MAX_OPERANDS);
            inst->num_operands = 0;
            add_operand(inst, exists, OPERAND_VARIABLE);
        } else {
            table_insert(builder->expressions, encoded, inst->assignee);
        }
    }

    vector_push_back(&current_block->instructions, inst);
}

static Instruction *previous_instruction(IRBuilder *builder)
{
    BasicBlock *current_block = builder->current_block;
    if (!current_block)
        error("no block to get previous instruction from");

    Instruction *inst = (Instruction *)vector_get(
            &current_block->instructions,
            current_block->instructions.size - 1
            );
    return inst;
}

static void add_operand(Instruction *inst, void *value, OperandKind kind)
{
    if (inst->num_operands == MAX_OPERANDS)
        error("too many operands for instruction %d", inst->opcode);

    Operand operand = { .kind = kind };
    switch (operand.kind) {
        case OPERAND_LITERAL:
            operand.literal = *((Literal *)value);
            break;
        case OPERAND_VARIABLE:
            operand.var = (char *)value;
            break;
        case OPERAND_LABEL:
            operand.label = (char *)value;
            break;
        default: error("invalid OperandKind: %d", kind);
    }

    inst->operands[inst->num_operands++] = operand;
}

static void add_operands_from_node(IRBuilder *builder, Instruction *inst, ASTNode *node)
{
    switch (node->kind) {
        case NODE_LITERAL_EXPR:
            add_operand(inst, &node->literal, OPERAND_LITERAL);
            break;
        case NODE_REF_EXPR:
            add_operand(inst, node->ref, OPERAND_VARIABLE);
            break;
        default:
            emit(builder, node);
            Instruction *temporary = previous_instruction(builder);
            add_operand(inst, temporary->assignee, OPERAND_VARIABLE);
    }
}

static void emit_function(IRBuilder *builder, FuncDecl func)
{
    add_block(builder, func.name);

    Instruction *inst = make_instruction(OP_DEF);
    add_operand(inst, func.name, OPERAND_LABEL);

    add_instruction(builder, inst);

    emit(builder, func.params);
    emit(builder, func.body);
}

static void emit_variable(IRBuilder *builder, VarDecl var)
{
    Instruction *inst = make_instruction(OP_ASSIGN);
    inst->assignee = var.name;

    table_insert(builder->var_names, var.name, var.name);

    add_operands_from_node(builder, inst, var.init);
    add_instruction(builder, inst);
}

static void emit_assignment(IRBuilder *builder, AssignStmt assign)
{
    Instruction *inst = make_instruction(OP_ASSIGN);
    inst->assignee = assign.name;

    add_operands_from_node(builder, inst, assign.value);
    add_instruction(builder, inst);
}

static void emit_return(IRBuilder *builder, RetStmt ret)
{
    Instruction *inst = make_instruction(OP_RET);
    add_operands_from_node(builder, inst, ret.value);
    add_instruction(builder, inst);
}

char *create_temporary_name(IRBuilder *builder)
{
    // Generate a random 6 character string for the temporary variable
    char *temporary_name = NULL;

    do {
        temporary_name = rand_str(6);
        if (!table_lookup(builder->var_names, temporary_name))
            break;
        free(temporary_name);
    } while (!temporary_name);

    return temporary_name;
}

static void emit_unary(IRBuilder *builder, UnaryExpr unary)
{
    Instruction *inst = make_instruction((OpCode)unary.un_op);
    inst->assignee = create_temporary_name(builder);

    add_operands_from_node(builder, inst, unary.expr);
    add_instruction(builder, inst);
}

static void emit_binary(IRBuilder *builder, BinaryExpr binary)
{
    Instruction *inst = make_instruction((OpCode)binary.bin_op);
    inst->assignee = create_temporary_name(builder);

    ASTNode *exprs[2] = {binary.lhs, binary.rhs};
    for (uint8_t i = 0; i < 2; i++) {
        ASTNode *expr = exprs[i];
        add_operands_from_node(builder, inst, expr);
    }

    add_instruction(builder, inst);
}

static ASTNode *emit(IRBuilder *builder, ASTNode *node)
{
    if (!node) return NULL;

    node->visited = true;
    ASTNode *next = node->next;

    switch (node->kind) {
        case NODE_NOOP:
            free(node);
            break;
        case NODE_FUNC_DECL:
            emit_function(builder, node->func_decl);
            break;
        case NODE_VAR_DECL:
            emit_variable(builder, node->var_decl);
            break;
        case NODE_ASSIGN_STMT:
            emit_assignment(builder, node->assign);
            break;
        case NODE_COND_STMT:
            error("conditional translation to IR is not implemented yet");
            break;
        case NODE_RET_STMT:
            emit_return(builder, node->ret_stmt);
            break;
        case NODE_UNARY_EXPR:
            emit_unary(builder, node->unary);
            break;
        case NODE_BINARY_EXPR:
            emit_binary(builder, node->binary);
            break;
        case NODE_LITERAL_EXPR: // Leaf node
        case NODE_REF_EXPR:
            return node;
        default: error("cannot emit IR from node: %d", node->kind);
    }

    emit(builder, next);
    return NULL;
}

Program emit_ir(ASTNode *root)
{
    IRBuilder builder = {0};
    ir_builder_init(&builder);

    Program program = {0};
    add_block(&builder, "$entry");
    program.entry = builder.current_block;

    emit(&builder, root);

    add_block(&builder, "$exit");
    program.exit  = builder.current_block;

    program.blocks = builder.blocks;
    program.num_blocks = builder.block_count;

    ir_builder_free(&builder);
    return program;
}

void dump_operand(Operand operand)
{
    switch (operand.kind) {
        case OPERAND_LITERAL:
            dump_literal(operand.literal);
            break;
        case OPERAND_VARIABLE:
            printf("%s", operand.var);
            break;
        case OPERAND_LABEL:
            printf("%s", operand.label);
            break;
        default: error("invalid OperandKind: %d", operand.kind);
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
        default: error("invalid OpCode %d", opcode);
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
        default: error("invalid Instruction: %d", inst->opcode);
    }
    printf("\n");
}

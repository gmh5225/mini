#include "ir.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct IRBuilder IRBuilder;
struct IRBuilder
{
    int in_condition;
    int in_compound;
    int in_assignment;
    Instruction *prev_inst;
    Table *register_names;
    BasicBlock *blocks;
    BasicBlock *current_block;
    int block_count;
};

/* IRBuilder */
static void ir_builder_init(IRBuilder *builder)
{
    builder->in_condition = 0;
    builder->in_compound = 0;
    builder->in_assignment = 0;
    builder->prev_inst = NULL;
    builder->register_names = table_new();
    builder->blocks = builder->current_block = NULL;
    builder->block_count = 0;
}

static void ir_builder_free(IRBuilder *builder)
{
    table_free(builder->register_names);
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

static void add_instruction(IRBuilder *builder, Instruction *inst)
{
    BasicBlock *current_block = builder->current_block;
    if (!current_block)
        error("no block to add instruction to");

    vector_push_back(&current_block->instructions, inst);
}

static bool is_compound(ASTNode *node)
{
    if (node->kind == NODE_LITERAL_EXPR || node->kind == NODE_REF_EXPR)
        return false;
    return true;
}

static Instruction *previous_instruction(IRBuilder *builder)
{
    Instruction *previous = (Instruction *)vector_get(
            &builder->current_block->instructions,
            builder->current_block->instructions.size - 1
            );
    return previous;
}

static void emit(IRBuilder *builder, ASTNode *node)
{
    if (!node) return;

    ASTNode *next = node->next;
    Instruction *inst = NULL;

    switch (node->kind) {
        case NODE_FUNC_DECL:
            add_block(builder, node->func_decl.name);

            inst = make_instruction(OP_DEF);

            inst->operands[0].kind = OPERAND_LABEL;
            inst->operands[0].label_name = node->func_decl.name;

            inst->num_operands = 1;

            add_instruction(builder, inst);

            emit(builder, node->func_decl.params);
            emit(builder, node->func_decl.body);
            break;
        case NODE_VAR_DECL:
            inst = make_instruction(OP_LOAD);

            inst->operands[0].kind = OPERAND_REGISTER;
            inst->operands[0].reg_name = node->var_decl.name;

            inst->num_operands = 1;

            add_instruction(builder, inst);

            if (node->var_decl.init) {
                Instruction *store_inst = make_instruction(OP_STORE);

                store_inst->operands[0].kind = OPERAND_REGISTER;
                store_inst->operands[0].reg_name = node->assign.name;

                if (is_compound(node->var_decl.init)) {
                    emit(builder, node->var_decl.init);
                    Instruction *previous = previous_instruction(builder);
                    store_inst->operands[1].kind = OPERAND_REGISTER;
                    store_inst->operands[1].reg_name = previous->operands[0].reg_name;
                }
                else {
                    switch (node->var_decl.init->kind) {
                        case NODE_LITERAL_EXPR:
                            store_inst->operands[1].kind = OPERAND_LITERAL;
                            store_inst->operands[1].literal = node->var_decl.init->literal;
                            break;
                        case NODE_REF_EXPR:
                            store_inst->operands[1].kind = OPERAND_REGISTER;
                            store_inst->operands[1].reg_name = node->var_decl.init->ref;
                            break;
                        default: error("unexpected compound node: %d",
                                         node->var_decl.init->kind);
                    }
                }
                add_instruction(builder, store_inst);
            }
            break;
        case NODE_ASSIGN_STMT:
            inst = make_instruction(OP_STORE);

            inst->operands[0].kind = OPERAND_REGISTER;
            inst->operands[0].reg_name = node->assign.name;

            if (is_compound(node->assign.value)) {
                emit(builder, node->assign.value);
                Instruction *previous = previous_instruction(builder);
                inst->operands[1].kind = OPERAND_REGISTER;
                inst->operands[1].reg_name = previous->operands[0].reg_name;
            }
            else {
                switch (node->assign.value->kind) {
                    case NODE_LITERAL_EXPR:
                        inst->operands[1].kind = OPERAND_LITERAL;
                        inst->operands[1].literal = node->assign.value->literal;
                        break;
                    case NODE_REF_EXPR:
                        inst->operands[1].kind = OPERAND_REGISTER;
                        inst->operands[1].reg_name = node->assign.value->ref;
                        break;
                    default: error("unexpected compound node: %d",
                                     node->assign.value->kind);
                }
            }

            inst->num_operands = 2;

            add_instruction(builder, inst);
            break;
        case NODE_COND_STMT:
            error("conditional translations to IR are not implemented yet");
            break;
        case NODE_RET_STMT:
            const char *return_prefix = "ret."; // length 4
            char *block_tag = builder->current_block->tag;
            size_t block_tag_length = strlen(block_tag);

            size_t length = 4 + block_tag_length + 1;
            char *return_label = calloc(length, sizeof(char));
            memcpy(return_label, return_prefix, 4);
            memcpy(return_label + 4, block_tag, block_tag_length);

            inst = make_instruction(OP_RET);

            if (is_compound(node->ret_stmt.value)) {
                emit(builder, node->ret_stmt.value);
                Instruction *previous = previous_instruction(builder);
                inst->operands[0].kind = OPERAND_REGISTER;
                inst->operands[0].reg_name = previous->operands[0].reg_name;
            }
            else {
                switch (node->ret_stmt.value->kind) {
                    case NODE_LITERAL_EXPR:
                        inst->operands[0].kind = OPERAND_LITERAL;
                        inst->operands[0].literal = node->ret_stmt.value->literal;
                        break;
                    case NODE_REF_EXPR:
                        inst->operands[0].kind = OPERAND_REGISTER;
                        inst->operands[0].reg_name = node->ret_stmt.value->ref;
                        break;
                    default: error("unexpected compound node: %d",
                                     node->ret_stmt.value->kind);
                }
            }

            inst->num_operands = 1;

            add_block(builder, return_label);
            add_instruction(builder, inst);
            break;
        case NODE_UNARY_EXPR:
            inst = make_instruction((OpCode)node->unary.un_op);

            // x := -(a + b * c / d)
            if (is_compound(node->unary.expr)) {
                emit(builder, node->unary.expr);
                Instruction *previous = previous_instruction(builder);
                inst->operands[0].kind = OPERAND_REGISTER;
                inst->operands[0].reg_name = previous->operands[1].reg_name;
            }
            else {
                switch (node->unary.expr->kind) {
                    case NODE_LITERAL_EXPR:
                        inst->operands[0].kind = OPERAND_LITERAL;
                        inst->operands[0].literal = node->unary.expr->literal;
                        break;
                    case NODE_REF_EXPR:
                        inst->operands[0].kind = OPERAND_REGISTER;
                        inst->operands[0].reg_name = node->unary.expr->ref;
                        break;
                    default: error("unexpected compound node: %d",
                                     node->ret_stmt.value->kind);
                }
            }

            inst->num_operands = 1;

            add_instruction(builder, inst);
            break;
        case NODE_BINARY_EXPR:
            inst = make_instruction((OpCode)node->binary.bin_op);

            Operand *left = &inst->operands[0];
            Operand *right = &inst->operands[1];

            if (is_compound(node->binary.lhs)) {
                emit(builder, node->binary.lhs);
                Instruction *previous = previous_instruction(builder);
                left->kind = OPERAND_REGISTER;
                left->reg_name = previous->operands[0].reg_name;
            }
            else {
                switch (node->binary.lhs->kind) {
                    case NODE_LITERAL_EXPR:
                        left->kind = OPERAND_LITERAL;
                        left->literal = node->binary.lhs->literal;
                        break;
                    case NODE_REF_EXPR:
                        left->kind = OPERAND_REGISTER;
                        left->reg_name = node->binary.lhs->ref;
                        break;
                    default: error("unexpected compound node: %d",
                                     node->binary.lhs->kind);
                }
            }

            if (is_compound(node->binary.rhs)) {
                emit(builder, node->binary.rhs);
                Instruction *previous = previous_instruction(builder);
                right->kind = OPERAND_REGISTER;
                right->reg_name = previous->operands[0].reg_name;
            }
            else {
                switch (node->binary.rhs->kind) {
                    case NODE_LITERAL_EXPR:
                        right->kind = OPERAND_LITERAL;
                        right->literal = node->binary.rhs->literal;
                        break;
                    case NODE_REF_EXPR:
                        right->kind = OPERAND_REGISTER;
                        right->reg_name = node->binary.rhs->ref;
                        break;
                    default: error("unexpected compound node: %d",
                                     node->binary.rhs->kind);
                }
            }

            inst->num_operands = 2;

            add_instruction(builder, inst);
            break;
        case NODE_LITERAL_EXPR: // Leaf node
        case NODE_REF_EXPR:
            inst = make_instruction(OP_LOAD);

            // Generate a random 6 character string for the temporary register
            char *temporary_name = NULL;
            do {
                temporary_name = rand_str(6);
                if (!table_lookup(builder->register_names, temporary_name))
                    break;
                free(temporary_name);
            } while (!temporary_name);

            inst->operands[0].kind = OPERAND_REGISTER;
            inst->operands[0].reg_name = temporary_name;

            if (node->kind == NODE_LITERAL_EXPR) {
                inst->operands[1].kind = OPERAND_LITERAL;
                inst->operands[1].literal = node->literal;
            }
            else {
                inst->operands[1].kind = OPERAND_REGISTER;
                inst->operands[1].reg_name = node->ref;
            }

            add_instruction(builder, inst);
            return;
        default: error("cannot emit IR from node: %d", node->kind);
    }

    emit(builder, next);
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

void dump_instruction(Instruction *inst)
{
    switch (inst->opcode) {
        case OP_DEF:
            printf("def %s:\n", inst->operands[0].label_name);
            break;
        case OP_LOAD:
            printf("  load %s\n", inst->operands[0].reg_name);
            break;
        case OP_STORE:
            printf("  store %s ", inst->operands[0].reg_name);
            switch (inst->operands[1].kind) {
                case OPERAND_LITERAL:
                    // TODO: fix this
                    printf("%x\n", inst->operands[1].literal);
                    break;
                case OPERAND_REGISTER:
                    printf("%s\n", inst->operands[1].reg_name);
                    break;
                default: error("unexpected operand: %d", inst->operands[1].kind);
            }
            break;
        case OP_ADD:
            printf("  add %s %s\n",
                    inst->operands[0].reg_name,
                    inst->operands[1].reg_name);
            break;
        case OP_MUL:
            printf("  mul %s %s\n",
                    inst->operands[0].reg_name,
                    inst->operands[1].reg_name);
            break;
        case OP_RET:
            printf("  ret %s\n", inst->operands[0].reg_name);
            break;
        default: error("invalid Instruction! (%d)", inst->opcode);
    }
}

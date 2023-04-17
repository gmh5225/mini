#include "ir.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

typedef struct IRBuilder IRBuilder;
struct IRBuilder
{
    int cond;
    int comp;
    int assign;
    char *temporary;
    Table *register_names;
    BasicBlock *blocks;
    BasicBlock *current_block;
    int block_count;
};

/* IRBuilder */
static void ir_builder_init(IRBuilder *builder)
{
    builder->cond = builder->comp = builder->assign = 0;
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

static void emit(IRBuilder *builder, ASTNode *node)
{
    if (!node) return;

    ASTNode *next = node->next;
    Instruction *inst = NULL;

    switch (node->kind) {
        case NODE_FUNC_DECL:
            add_block(builder, node->func_decl.name);

            inst = make_instruction(OP_DEF);
            inst->operands[0].kind = OPERAND_TYPE;
            inst->operands[0].type = node->func_decl.return_type;

            inst->operands[1].kind = OPERAND_LABEL;
            inst->operands[1].label_name = node->func_decl.name;

            inst->num_operands = 2;

            add_instruction(builder, inst);

            emit(builder, node->func_decl.params);
            emit(builder, node->func_decl.body);
            break;
        case NODE_VAR_DECL:
            inst = make_instruction(OP_ALLOC);
            inst->operands[0].kind = OPERAND_TYPE;
            inst->operands[0].type = node->var_decl.type;

            inst->operands[1].kind = OPERAND_REGISTER;
            inst->operands[1].reg_name = node->var_decl.name;

            inst->num_operands = 2;

            add_instruction(builder, inst);
            break;
        case NODE_COND_STMT:
            break;
        case NODE_RET_STMT:
            const char *return_prefix = "$ret_"; // length 5
            char *block_tag = builder->current_block->tag;
            size_t block_tag_length = strlen(block_tag);

            size_t length = 5 + block_tag_length + 1;
            char *return_label = calloc(length, sizeof(char));
            memcpy(return_label, return_prefix, 5);
            memcpy(return_label + 5, block_tag, block_tag_length);
            return_label[length - 1] = 0;

            add_block(builder, return_label);

            inst = make_instruction(OP_RET);

            // If the expression for the return statement is a compound expression,
            // emit all instructions to create temporaries for the expression,
            // and make the operand for the instruction below be an OPERAND_REGISTER
            // referring to that temporary.
            if (is_compound(node->ret_stmt.value)) {
                emit(builder, node->ret_stmt.value);
            }
            else {

            }

            add_instruction(builder, inst);
            break;
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

}

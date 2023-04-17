#include "compile.h"
#include "codegen.h"
#include "ir.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//static ASTNode *convert_to_mir(IRBuilder *, ASTNode *);
//
//static bool is_flat_expression(NodeKind kind) {
//    return (kind == NODE_LITERAL_EXPR || kind == NODE_REF_EXPR) ? true : false;
//}
//
//static void flatten_expression_into_value(IRBuilder *builder, ASTNode *expr, Operand *value) {
//    builder->in_compound = true;
//    ASTNode *node = convert_to_mir(builder, expr);
//    switch (node->kind) {
//        case NODE_LITERAL_EXPR:
//            value->kind = OPERAND_LITERAL;
//            value->literal = node->literal;
//            break;
//        case NODE_REF_EXPR:
//            value->kind = OPERAND_REGISTER;
//            value->reg_name = node->ref;
//            break;
//        default: error("cannot emit assignment from unflattened node: %d", 
//                         node->kind);
//    }
//    builder->in_compound = false;
//}
//
//static void emit_assignment(IRBuilder *builder, char *var_name, ASTNode *var_value) {
//    builder->in_assignment = true;
//    MIR *inst = make_instruction(OP_STORE);
//    inst->operands[0].kind = OPERAND_REGISTER;
//    inst->operands[0].reg_name = var_name;
//
//    Operand value;
//    if (!is_flat_expression(var_value->kind)) {
//        flatten_expression_into_value(builder, var_value, &value);
//    }
//
//    inst->operands[1] = value;
//    inst->num_operands = 2;
//
//    BasicBlock *current_block = builder->current_block;
//    vector_push_back(&current_block->instructions, inst);
//
//    builder->in_assignment = false;
//}
//
//static void emit_variable(IRBuilder *builder, char *var_name, 
//        Type var_type, ASTNode *var_value) {
//    MIR *inst = make_instruction(OP_ALLOC);
//    inst->operands[0].kind = OPERAND_REGISTER,
//    inst->operands[0].reg_name = var_name,
//    inst->num_operands = 1;
//
//    table_insert(builder->register_names, var_name, var_name);
//    BasicBlock *current_block = builder->current_block;
//    vector_push_back(&current_block->instructions, inst);
//
//    if (var_value) {
//        emit_assignment(builder, var_name, var_value);
//    }
//}
//
//static void emit_unary(IRBuilder *builder, UnaryOp op, ASTNode *expr) {
//    Operand value;
//    if (!is_flat_expression(expr->kind)) {
//        flatten_expression_into_value(builder, expr, &value);
//    }
//
//    char *temp = NULL;
//    do {
//        temp = rand_str(6);
//        if (table_lookup(builder->register_names, temp)) {
//            free(temp);
//        }
//    } while (!temp);
//
//    table_insert(builder->register_names, temp, temp);
//    builder->temporary = temp;
//
//    OpCode opcode;
//    switch (op) {
//        case UN_NEG:
//            opcode = OP_NEG;
//            break;
//        case UN_NOT:
//            opcode = OP_NOT;
//            break;
//        case UN_DEREF:
//            break;
//        case UN_ADDR:
//            break;
//        default: error("cannot generate MIR from unary op: %d", op);
//    }
//
//    MIR *inst = make_instruction(opcode);
//    inst->operands[0] = value;
//}
//
//static void convert_to_mir(IRBuilder *builder, ASTNode *node) {
//    if (!node) return;
//
//    printf("converting node kind: %d\n", node->kind);
//
//    ASTNode *next = node->next;
//    switch (node->kind) {
//        case NODE_FUNC_DECL:
//            emit_function(builder,
//                    node->func_decl.name,
//                    node->func_decl.return_type,
//                    node->func_decl.params,
//                    node->func_decl.body);
//            break;
//        case NODE_VAR_DECL:
//            emit_variable(builder,
//                    node->var_decl.name,
//                    node->var_decl.type,
//                    node->var_decl.init);
//            break;
//        case NODE_ASSIGN_STMT:
//            emit_assignment(builder,
//                    node->assign.name,
//                    node->assign.value);
//            break;
//        case NODE_COND_STMT:
//            break;
//        case NODE_UNARY_EXPR:
//            emit_unary(builder,
//                    node->unary.un_op,
//                    node->unary.expr);
//            break;
//        case NODE_BINARY_EXPR:
//            break;
//        case NODE_LITERAL_EXPR:
//            break;
//        case NODE_REF_EXPR:
//            break;
//        default: error("cannot generate MIR from node: %d", node->kind);
//    }
//
//    convert_to_mir(builder, next);
//}
//
//int compile(ASTNode *program, char *output_filename) {
//    int status = 0;
//
//    IRBuilder builder = {0};
//    ir_builder_init(&builder);
//    convert_to_mir(&builder, program);
//    ir_builder_free(&builder);
//
//    CodeBuffer output = {0};
//    code_buffer_init(&output);
//    code_buffer_write_to_file(&output, output_filename);
//    code_buffer_free(&output);
//
//    return status;
//}

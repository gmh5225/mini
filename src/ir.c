#include "ir.h"
#include "util/table.h"
#include "util/util.h"

#include <stdlib.h>
#include <string.h>

//typedef struct {
//    bool in_condition;
//    char *curr_block_tag;
//    char *curr_node_name;
//    SSANode *nodes;
//    IRBlockArray *blocks;
//    Table *vars;
//} SSAContext;
//
//static SSANode *make_ssa_node(char *name, SSAValue value, SSAKind kind) {
//    SSANode *node = calloc(1, sizeof(SSANode));
//    node->sub = 0;
//    node->name = name;
//    node->kind = kind;
//    node->value = value;
//    node->next = NULL;
//    return node;
//}
//
//static void ssa_context_init(SSAContext *ctx, IRBlockArray *blocks) {
//    memset(ctx, 0, sizeof(*ctx));
//    ctx->blocks = blocks;
//    ctx->vars = table_new();
//}
//
//static void ssa_context_free(SSAContext *ctx) {
//    table_free(ctx->vars);
//}
//
//static void ssa_context_add_node(SSAContext *ctx, char *name, SSAValue value, SSAKind kind) {
//    SSANode *node = make_ssa_node(name, value, kind);
//    if (!ctx->nodes) {
//        ctx->nodes = node;
//        return;
//    }
//
//    SSANode *iter = ctx->nodes;
//    while (iter) {
//        // If the variable with that name already exists,
//        // increment the new target variable with its previous reference by 1
//        size_t len = strlen(name);
//        if (iter->name && (strlen(iter->name) == len) &&
//                memcmp(iter->name, name, len) == 0) {
//            node->sub = iter->sub + 1;
//        }
//
//        if (!iter->next) {
//            iter->next = node;
//            return;
//        }
//
//        iter = iter->next;
//    }
//}
//
//void ssa_context_add_block(SSAContext *ctx, char *tag) {
//    static int unique_id = 0;
//    BasicBlock block = {
//        .id = unique_id++,
//        .tag = tag,
//        .nodes = ctx->nodes,
//    };
//
//    ir_block_array_append(ctx->blocks, block);
//
//    ctx->nodes = NULL;
//    ctx->curr_node_name = NULL;
//}
//
//void translate_to_ssa(ControlFlowGraph *graph) {
//}

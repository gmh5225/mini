#include "minigen/ir.h"

#include <stdlib.h>
#include <string.h>

typedef struct minigen_ssa_node {
    int sub;
    char *name;
    void *value;
    struct minigen_ssa_node *next;
} SSANode;

typedef struct minigen_ssa_context {
    char *last_node_name;
    SSANode *nodes;
} SSAContext;

void ssa_context_init(SSAContext *ctx) {
    ctx->last_node_name = NULL;
    ctx->nodes = NULL;
}

void ssa_context_add_node(SSAContext *ctx, char *name, void *value) {
    SSANode *node = calloc(1, sizeof(SSANode));
    node->sub = 0;
    node->name = name;
    node->value = value;
    node->next = NULL;

    if (!ctx->nodes) {
        ctx->nodes = node;
        return;
    } 

    SSANode *iter = ctx->nodes;
    while (iter) {
        // If the variable with that name already exists,
        // increment the new target variable with its previous reference by 1
        size_t len = strlen(name);
        if ((strlen(iter->name) == len) &&
                memcmp(iter->name, name, len) == 0) {
            node->sub = iter->sub + 1;
        }

        if (!iter->next) {
            iter->next = node;
            return;
        }

        iter = iter->next;
    }
}

//void translate_to_ssa(SSAContext *ctx, ast_node *root) {
//    if (!root) return;
//
//    switch (root->kind) {
//        case NODE_ASSIGN_EXPR:
//            ssa_context_add_node(ctx, root->assign.name, root->assign.value);
//            break;
//        case NODE_VAR_DECL:
//            if (root->var_decl.init) {
//                ctx->last_node_name = root->var_decl.name;
//                translate_to_ssa(ctx, root->var_decl.init);
//            }
//            break;
//        case NODE_FUNC_DECL:
//            translate_to_ssa(ctx, root->func_decl.params);
//            translate_to_ssa(ctx, root->func_decl.body);
//            break;
//        case NODE_REF_EXPR:
//            ssa_context_add_node(ctx, ctx->last_node_name, root);
//            ctx->last_node_name = NULL;
//            break;
//        case NODE_LITERAL_EXPR:
//            ssa_context_add_node(ctx, ctx->last_node_name, root);
//            ctx->last_node_name = NULL;
//            break;
//        case NODE_UNARY_EXPR:
//            ssa_context_add_node(ctx, ctx->last_node_name, root);
//            ctx->last_node_name = NULL;
//            break;
//        case NODE_BINARY_EXPR:
//            ssa_context_add_node(ctx, ctx->last_node_name, root);
//            ctx->last_node_name = NULL;
//            break;
//        default: break;
//    }
//
//    translate_to_ssa(ctx, root->next);
//}

//void optimize(ast_node *program) {
//    // Translate to SSA form
//    SSAContext ctx = {0};
//    translate_to_ssa(&ctx, program);
//
//    printf("Translated to SSA form:\n");
//    SSANode *iter = ctx.nodes;
//    while (iter) {
//        printf("[SSANode %s#%d]\n", iter->name, iter->sub);
//        dump_ast(iter->value);
//        iter = iter->next;
//    }
//}

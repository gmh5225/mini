#include "mini.h"

// TODO: Implement Dead-code elimination, Constant folding

typedef void (*OptimizeFunc)(Node *);

typedef struct {
    NodeKind on;
    OptimizeFunc fn;
} Optimizer;

void fold_constants(Node *root) {
}

static Optimizer optimizers[] = {
    { NODE_BINARY_EXPR, fold_constants },
};

typedef struct ssa_item ssa_item;
struct ssa_item {
    char *name;
    int sub;
    Node *value;
    ssa_item *next;
};

typedef struct {
    char *last_var_name;
    ssa_item *items;
} ssa_context;

void ssa_context_init(ssa_context *ctx) {
    ctx->last_var_name = NULL;
    ctx->items = NULL;
}

void ssa_context_add_item(ssa_context *ctx, char *name, Node *value) {
    ssa_item *item = calloc(1, sizeof(ssa_item));
    item->sub = 0;
    item->name = name;
    item->value = value;
    item->next = NULL;

    if (!ctx->items) {
        ctx->items = item;
        return;
    } 

    ssa_item *iter = ctx->items;
    while (iter) {
        // If the variable with that name already exists,
        // increment the new target variable with its previous reference by 1
        size_t len = strlen(name);
        if ((strlen(iter->name) == len) &&
                memcmp(iter->name, name, len) == 0) {
            item->sub = iter->sub + 1;
            printf("incremented %s count by 1: %d\n", item->name, item->sub);
        }

        if (!iter->next) {
            iter->next = item;
            return;
        }

        iter = iter->next;
    }
}

//void ssa_context_update_item(ssa_context *ctx, char *name, Node *value) {
//  ssa_item *iter = ctx->items;
//    for (;;) {
//        if (!iter->next) break;
//
//        size_t len = strlen(name);
//        if ((strlen(iter->name) == len) &&
//            memcmp(iter->name, name, len) == 0) {
//            iter->value = value;
//            return;
//        }
//        iter = iter->next;
//    }
//}

void translate_to_ssa(ssa_context *ctx, Node *root) {
    if (!root) return;

    switch (root->kind) {
        case NODE_ASSIGN_EXPR:
            ssa_context_add_item(ctx, root->assign.name, root->assign.value);
            break;
        case NODE_VAR_DECL:
            if (root->var_decl.init) {
                ctx->last_var_name = root->var_decl.name;
                translate_to_ssa(ctx, root->var_decl.init);
            }
            break;
        case NODE_FUNC_DECL:
            translate_to_ssa(ctx, root->func_decl.params);
            translate_to_ssa(ctx, root->func_decl.body);
            break;
        case NODE_LITERAL_EXPR:
            ssa_context_add_item(ctx, ctx->last_var_name, root);
            ctx->last_var_name = NULL;
            break;
        case NODE_UNARY_EXPR:
            ssa_context_add_item(ctx, ctx->last_var_name, root);
            ctx->last_var_name = NULL;
            break;
        case NODE_BINARY_EXPR:
            ssa_context_add_item(ctx, ctx->last_var_name, root);
            ctx->last_var_name = NULL;
            break;
        default: break;
    }

    translate_to_ssa(ctx, root->next);
}

void optimize(Node *program) {
    // Translate to SSA form
    ssa_context ctx = {0};
    translate_to_ssa(&ctx, program);

    printf("Translated to SSA form:\n");
    ssa_item *iter = ctx.items;
    while (iter) {
        printf("[ssa-item %s#%d]\n", iter->name, iter->sub);
        dump_ast(iter->value);
        iter = iter->next;
    }
}

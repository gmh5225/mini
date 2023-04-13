#include "codegen.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *curr_block_tag;
    char *curr_node_name;
    SSANode *nodes;
    BasicBlock *blocks;
} SSAContext;

void ssa_context_add_node(SSAContext *ctx, char *name, ASTNode *value) {
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
        if (iter->name && (strlen(iter->name) == len) &&
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

void ssa_context_add_block(SSAContext *ctx, char *tag) {
    static int unique_id = 0;

    BasicBlock *block = calloc(1, sizeof(BasicBlock));
    block->id = unique_id++;
    block->tag = tag;
    block->nodes = ctx->nodes;
    block->next = NULL;

    if (!ctx->blocks) {
        ctx->blocks = block;
    } else {
        BasicBlock *iter = ctx->blocks;
        while (iter) {
            if (!iter->next) break;
            iter = iter->next;
        }
        iter->next = block;
    }

    ctx->nodes = NULL;
    ctx->curr_node_name = NULL;
}

static void translate_to_ssa_impl(SSAContext *ctx, ASTNode *root) {
    if (!root) return;

    switch (root->kind) {
        case NODE_FUNC_DECL:
            ctx->curr_block_tag = root->func_decl.name;
            translate_to_ssa_impl(ctx, root->func_decl.params);
            translate_to_ssa_impl(ctx, root->func_decl.body);
            break;
        case NODE_VAR_DECL:
            ctx->curr_node_name = root->var_decl.name;
            translate_to_ssa_impl(ctx, root->var_decl.init);
            break;
        case NODE_RET_STMT:
            ssa_context_add_block(ctx, ctx->curr_block_tag);
            break;
        case NODE_COND_STMT:
            ssa_context_add_block(ctx, ctx->curr_block_tag);
            translate_to_ssa_impl(ctx, root->cond_stmt.expr);
            translate_to_ssa_impl(ctx, root->cond_stmt.body);
            break;
        case NODE_ASSIGN_EXPR:
            ssa_context_add_node(ctx, root->assign.name, root->assign.value);
            break;
        case NODE_UNARY_EXPR:
        case NODE_BINARY_EXPR:
        case NODE_LITERAL_EXPR:
        case NODE_REF_EXPR:
            // TODO: Change the SSAContext to include all previous variable references
            // in this scope. If we found a NODE_REF_EXPR, change the name of the .ref
            // member to be the latest node name.
            ssa_context_add_node(ctx, ctx->curr_node_name, root);
            break;
        default: break;
    }

    translate_to_ssa_impl(ctx, root->next);
}

BasicBlock *translate_to_ssa(ASTNode *program) {
    SSAContext ctx = {0};
    translate_to_ssa_impl(&ctx, program);
    return ctx.blocks;
}

void code_buffer_init(CodeBuffer *code) {
    code->buffer = calloc(DEFAULT_TARGET_CODE_CAPACITY, sizeof(char));
    code->code_length = 0;
    code->code_capacity = DEFAULT_TARGET_CODE_CAPACITY;
}

void code_buffer_write_bytes(CodeBuffer *code, char *bytes, size_t length) {
    if (code->code_length + length >= code->code_capacity) {
        code->code_capacity <<= 1;
        void *tmp = realloc(code->buffer, sizeof(char) * code->code_capacity);
        code->buffer = tmp;
    }
    memcpy(code->buffer + code->code_length, bytes, length);
    code->code_length += length;
}

void code_buffer_write_to_file(CodeBuffer *code, char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        error("couldn't open file: `%s`", filename);
    }

    size_t nwritten = fwrite(code->buffer, sizeof(char), code->code_length, f);
    if (nwritten != code->code_length) {
#ifdef DEBUG
        error("only wrote %zu/%zu bytes to file `%s`",
                nwritten, code->code_length, filename);
#endif
    }
    fclose(f);
}

void code_buffer_free(CodeBuffer *code) {
    if (code->buffer) {
        free(code->buffer);
        code->buffer = NULL;
    }
    code->code_length = code->code_capacity = 0;
}

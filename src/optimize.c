#include "optimize.h"
#include "types.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

void fold_constants(ASTNode *node)
{
    if (!node) return;

    ASTNode *next = node->next;

    switch (node->kind) {
        case NODE_FUNC_DECL:
            fold_constants(node->func_decl.body);
            break;
        case NODE_ASSIGN_STMT:
            AssignStmt assign = node->assign;
            if (assign.value->kind == NODE_REF_EXPR && 
                    strcmp(assign.name, assign.value->ref) == 0) {

#ifdef DEBUG
                LOG_INFO("elminiating self-assignment of variable `%s` on line %d, col %d",
                        assign.name, node->line, node->col);
#endif

                node->kind = NODE_NOOP;
                free(assign.value);
            }
            break;
        case NODE_BINARY_EXPR:
            ASTNode *lhs = node->binary.lhs;
            ASTNode *rhs = node->binary.rhs;

            if (lhs->kind == NODE_LITERAL_EXPR && lhs->kind == rhs->kind) {
                node->kind = NODE_LITERAL_EXPR;

                switch (node->binary.bin_op) {

                }
            }
            break;
    }

    fold_constants(next);
}

#include "optimize.h"
#include "types.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

void fold_constants(ASTNode *node)
{
    switch (node->kind) {
        case NODE_VAR_DECL:
            VarDecl var = node->var_decl;
            // num = num;
            if (var.init->kind == NODE_REF_EXPR &&
                    strcmp(var.name, var.init->ref) == 0) {
                node->kind = NODE_NOOP;
                free(var.init);
            }
            break;
        case NODE_ASSIGN_STMT:
            AssignStmt assign = node->assign;
            if (assign.value->kind == NODE_REF_EXPR &&
                    strcmp(assign.name, assign.value->ref) == 0) {
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
        default: break;
    }
}

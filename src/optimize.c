#include "optimize.h"
#include "types.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

intmax_t fold_int(BinaryOp op, intmax_t left, intmax_t right)
{
  intmax_t result = 0;
  switch (op) {
    case BIN_ADD:
      result = left + right;
      break;
    case BIN_SUB:
      result = left - right;
      break;
    case BIN_MUL:
      result = left * right;
      break;
    case BIN_DIV:
      result = left / right;
      break;
    case BIN_CMP:
      result = left == right;
      break;
    case BIN_CMP_NOT:
      result = left != right;
      break;
    case BIN_CMP_LT:
      result = left < right;
      break;
    case BIN_CMP_GT:
      result = left > right;
      break;
    case BIN_CMP_LT_EQ:
      result = left <= right;
      break;
    case BIN_CMP_GT_EQ:
      result = left >= right;
      break;
    default: fatal("unknown BinaryOp in fold_int: %d", op);
  }
  return result;
}

// Performs Constant Folding and Common-Subexpression Elimination in one pass
void fold_constants(Node *node)
{
  if (!node) return;

  Node *next = node->next;

  switch (node->kind) {
    case NODE_FUNC_DECL:
      fold_constants(node->func_decl.body);
      break;
    case NODE_VAR_DECL:
      fold_constants(node->var_decl.init);
      break;
    case NODE_ASSIGN_STMT:
      AssignStmt assign = node->assign;
      if (assign.value->kind == NODE_REF_EXPR &&
          strcmp(assign.name, assign.value->ref) == 0) {
        LOG_INFO("elminiating self-assignment of variable `%s` on line %d, col %d",
            assign.name, node->line, node->col);
        node->kind = NODE_NOOP;
        free(assign.value);
      } else {
        fold_constants(node->assign.value);
      }
      break;
    case NODE_UNARY_EXPR:
      fold_constants(node->unary.expr);
      break;
    case NODE_BINARY_EXPR:
      BinaryOp op = node->binary.bin_op;
      Node *lhs = node->binary.lhs;
      Node *rhs = node->binary.rhs;

      // TODO: We only fold literal constant expressions that are of the same type.
      // No implicit type coercion happens here. Add a warning down the line if the
      // types of rhs and lhs are not the same.

      if (lhs->kind == NODE_UNARY_EXPR || lhs->kind == NODE_BINARY_EXPR)
        fold_constants(lhs);

      if (rhs->kind == NODE_UNARY_EXPR || rhs->kind == NODE_BINARY_EXPR)
        fold_constants(rhs);

      if (lhs->kind == NODE_LITERAL_EXPR
          && lhs->kind == rhs->kind
          && lhs->literal.kind == rhs->literal.kind) {
        LOG_INFO("folding constant binary expression of on line %d, col %d",
            node->line, node->col);

        Value folded = { .kind = lhs->literal.kind };
        switch (folded.kind) {
          case VAL_INT:
            folded.i_val = fold_int(op, lhs->literal.i_val, rhs->literal.i_val);
            break;
          default: 
            LOG_WARN("constant folding not yet supported for Literal Type: %d", folded.kind);
            goto end;
        }

        node->kind = NODE_LITERAL_EXPR;
        node->literal = folded;
      }
      break;
  }

end:
  fold_constants(next);
}


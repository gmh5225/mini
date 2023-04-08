#include "ast.h"

#include <stdio.h>

const char *unary_op_strings[] = {
    [UNARY_BITWISE_NOT] = "~",
    [UNARY_LOGICAL_NOT] = "!",
    [UNARY_NEGATE] = "-",
    [UNARY_DEREFERENCE] = "*",
    [UNARY_ADDRESS] = "&",
};

const char *binary_op_strings[] = {
    [BINARY_ADD] = "+",
    [BINARY_SUBTRACT] = "-",
    [BINARY_MULTIPLY] = "*",
    [BINARY_DIVIDE] = "/",
    [BINARY_MODULUS] = "%",
    [BINARY_EQUALS] = "==",
    [BINARY_NOT_EQUALS] = "!=",
    [BINARY_GREATER_THAN] = ">",
    [BINARY_GREATER_THAN_EQ] = ">=",
    [BINARY_LESS_THAN] = "<",
    [BINARY_LESS_THAN_EQ] = "<=",
};

const char *type_strings[] = {
    [TYPE_UNKNOWN] = "[UNKNOWN TYPE]",
    [TYPE_VOID] = "void",
    [TYPE_BOOL] = "bool",
    [TYPE_INT] = "int",
    [TYPE_UINT] = "uint",
    [TYPE_FLOAT] = "float",
    [TYPE_CHAR] = "char",
    [TYPE_CUSTOM] = "[CUSTOM TYPE]",
};

const char *node_strings[] = {
    [NODE_PROGRAM] = "[PROGRAM]",
    [NODE_TYPEDEF] = "[TYPEDEF]",
    [NODE_BLOCK] = "[BLOCK]",
    [NODE_VARIABLE_DECLARATION] = "[VARIABLE_DECLARATION]",
    [NODE_VARIABLE_ASSIGNMENT] = "[VARIABLE_ASSIGNMENT]",
    [NODE_FUNCTION_DECLARATION] = "[FUNCTION_DECLARATION]",
    [NODE_FUNCTION_CALL] = "[FUNCTION_CALL]",
    [NODE_CONDITIONAL] = "[CONDITIONAL]",
    [NODE_EXPRESSION] = "[EXPRESSION]",
    [NODE_RETURN_STATEMENT] = "[RETURN_STATEMENT]",
    [NODE_LITERAL] = "[LITERAL]",
};

const char *unary_as_str(unary_op op) {
    return unary_op_strings[op];
}

const char *binary_as_str(binary_op op) {
    return binary_op_strings[op];
}

const char *type_as_str(type_kind kind) {
    return type_strings[kind];
}

const char *node_as_str(ast_node_type type) {
    return node_strings[type];
}

#define TEE "\u251C"  // ├
#define HOR "\u2500"  // ─
#define VER "\u2502"  // │

void print_function(function *function);
void print_variable(variable *variable);
void print_type(type_info *type);
void print_expression(expression *expression);
void print_unary_expression(unary_expr *unary);
void print_binary_expression(binary_expr *binary);

void ast_dump_impl(ast_node *root, int level) {
    if (!root) return;

    for (int i = 0; i < level; i++) {
        if (i == level - 1) {
            printf(TEE);
            printf(HOR);
        } else {
            printf(VER);
        }
    }

    printf("%s: ", node_as_str(root->type));
    switch (root->type) {
        case NODE_VARIABLE_DECLARATION:
            print_variable(&AS_VAR(root));
            break;
        case NODE_FUNCTION_DECLARATION:
            print_function(&AS_FUNC(root));
            break;
        case NODE_RETURN_STATEMENT:
            printf("expression = ");
            print_expression(&AS_EXPR(root));
            break;
        default:
            break;
    }

    if (root->num_children > 0) {
        printf("\n");
    }

    for (size_t i = 0; i < root->num_children; i++) {
        if (i == root->num_children - 1) {
            ast_dump_impl(root->children[i], level + 1);
        } else {
            ast_dump_impl(root->children[i], level + 1);
            printf("\n");
        }
    }
}

void ast_dump(ast_node *root) {
    ast_dump_impl(root, 0);
    printf("\n");
}

void print_type(type_info *type) {
    if (type->kind < TYPE_CUSTOM) {
        printf("%s", type_as_str(type->kind));
    } else {
        printf("%s", type->name);
    }
}

void print_function(function *function) {
    printf("name = \"%s\", return_type = ", function->name);
    print_type(&function->return_type);

    printf(", parameters = [");
    function_param *param = function->parameters;
    while (param) {
        printf("(name = \"%s\", type = ", param->name);
        print_type(&param->type);
        printf(")");
        param = param->next;
        if (param) {
            printf(", ");
        }
    }
    printf("]");
}

void print_variable(variable *variable) {
    printf("name = \"%s\", type = ", variable->name);
    print_type(&variable->type);

    printf(", assignment = ");
    if (variable->assignment) {
        print_expression(variable->assignment);
    } else {
        printf("nil");
    }
}

void print_expression(expression *expression) {
    switch (expression->type) {
        case EXPR_BOOL:
            printf("BOOL(%s)", expression->as.boolean ? "true" : "false");
            break;
        case EXPR_INT:
            printf("INT(%d)", expression->as.integer);
            break;
        case EXPR_UNARY:
            print_unary_expression(&expression->as.unary);
            break;
        case EXPR_BINARY:
            print_binary_expression(&expression->as.binary);
            break;
        case EXPR_VAR:
            printf("VAR(%s)", expression->as.string.data);
            break;
        default: 
            printf("[UNKNOWN EXPR TYPE]");
    }

}

void print_unary_expression(unary_expr *unary) {
    printf("Unary(%s", unary_as_str(unary->op));
    print_expression(unary->expr);
    printf(")");
}

void print_binary_expression(binary_expr *binary) {
    printf("Binary(");
    print_expression(binary->lhs);
    printf(" %s ", binary_as_str(binary->op));
    print_expression(binary->rhs);
    printf(")");
}

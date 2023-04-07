#include "ast.h"

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
    [NODE_BLOCK] = "[BLOCK]",
    [NODE_VARIABLE] = "[VARIABLE]",
    [NODE_FUNCTION] = "[FUNCTION]",
    [NODE_FUNCTION_CALL] = "[FUNCTION_CALL]",
    [NODE_CONDITIONAL] = "[CONDITIONAL]",
    [NODE_EXPRESSION] = "[EXPRESSION]",
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

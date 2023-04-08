#ifndef MINI_AST_H
#define MINI_AST_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    UNARY_BITWISE_NOT,      // ~
    UNARY_LOGICAL_NOT,      // !
    UNARY_NEGATE,           // -
    UNARY_DEREFERENCE,      // *
    UNARY_ADDRESS,          // &
} unary_op;

typedef enum {
    BINARY_ADD,             // +
    BINARY_SUBTRACT,        // -
    BINARY_MULTIPLY,        // *
    BINARY_DIVIDE,          // /
    BINARY_MODULUS,         // %
    BINARY_EQUALS,          // ==
    BINARY_NOT_EQUALS,      // !=
    BINARY_GREATER_THAN,    // >
    BINARY_GREATER_THAN_EQ, // >=
    BINARY_LESS_THAN,       // <
    BINARY_LESS_THAN_EQ,    // <=
} binary_op;

typedef enum {
    TYPE_UNKNOWN,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_CUSTOM,
} type_kind;

typedef struct type_info type_info;
typedef struct variable variable;
typedef struct function_param function_param;
typedef struct function function;
typedef struct expression expression;
typedef struct ast_node ast_node;

struct type_info {
    type_kind kind;
    char *name;
};

struct variable {
    char *name;
    type_info type;
    bool is_pointer;
    expression *assignment;
};

struct function_param {
    char *name;
    type_info type;
    struct function_param *next;
};

struct function {
    char *name;
    function_param *parameters;
    type_info return_type;
};

typedef enum {
    EXPR_UNKNOWN,
    EXPR_BOOL,
    EXPR_INT,
    EXPR_UINT,
    EXPR_FLOAT,
    EXPR_CHAR,
    EXPR_STRING,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_VAR,
    EXPR_FUNC_CALL,
    EXPR_EMPTY,
} expression_type;

typedef struct {
    unary_op op;
    expression *expr;
} unary_expr;

typedef struct {
    binary_op op;
    expression *lhs, *rhs;
} binary_expr;

struct expression {
    expression_type type;
    union {
        bool boolean;
        int integer;
        unsigned int unsigned_integer;
        double floating_point;
        char character;
        struct {
            char *data;
            size_t length;
        } string;
        unary_expr unary;
        binary_expr binary;
    } as;
};

#define AST_CHILDREN_CAPACITY 16

typedef enum {
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_VARIABLE,
    NODE_FUNCTION,
    NODE_FUNCTION_CALL,
    NODE_CONDITIONAL,
    NODE_EXPRESSION,
    NODE_RETURN_STATEMENT,
    NODE_LITERAL,
} ast_node_type;

typedef struct ast_node {
    ast_node_type type;
    union {
        variable variable;
        function function;
        expression expression;
    } as;
    struct ast_node **children;
    size_t num_children;
    size_t child_capacity;
} ast_node;

#define AS_VAR(node) ((*node).as.variable)
#define AS_FUNC(node) ((*node).as.function)
#define AS_EXPR(node) ((*node).as.expression)

extern const char *unary_op_strings[];
extern const char *binary_op_strings[];
extern const char *type_strings[];
extern const char *node_strings[];

const char *unary_as_str(unary_op op);
const char *binary_as_str(binary_op op);
const char *type_as_str(type_kind kind);
const char *node_as_str(ast_node_type type);

void ast_dump(ast_node *ast);

#endif

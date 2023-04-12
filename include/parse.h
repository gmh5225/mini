#ifndef MINI_PARSE_H
#define MINI_PARSE_H

#include "lex.h"
#include "types.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct ASTNode ASTNode;

typedef struct {
    char *name;
    Type return_type;
    ASTNode *params; 
    ASTNode *body;
} FuncDecl;

typedef struct {
    char *name;
    Type type;
    ASTNode *init;
} VarDecl;

typedef struct {
    char *name;
    ASTNode *value;
} AssignExpr;

typedef struct {
    ASTNode *value;
} RetStmt;

typedef struct {
    char un_op;
    ASTNode *expr;
} UnaryExpr;

typedef struct {
    char bin_op;
    ASTNode *lhs;
    ASTNode *rhs;
} BinaryExpr;

typedef union {
    intmax_t i_val;
    uintmax_t u_val;
    float f_val;
    double d_val;
    char c_val;
    bool b_val;
} Literal;

typedef enum {
    NODE_UNKNOWN,
    NODE_FUNC_DECL,
    NODE_VAR_DECL,
    NODE_IF_STMT,
    NODE_RET_STMT,
    NODE_FUNC_CALL_EXPR,
    NODE_ASSIGN_EXPR,
    NODE_UNARY_EXPR,
    NODE_BINARY_EXPR,
    NODE_LITERAL_EXPR,
    NODE_REF_EXPR,
} NodeKind;

struct ASTNode {
    NodeKind kind;
    ASTNode *next;
    Type type;
    union {
        FuncDecl func_decl;
        VarDecl var_decl;
        RetStmt ret_stmt;
        AssignExpr assign;
        UnaryExpr unary;
        BinaryExpr binary;
        Literal literal;
        char *ref;
    };
};

ASTNode *parse(TokenStream *stream);
void dump_ast(ASTNode *program);

#endif

#ifndef MINI_PARSE_H
#define MINI_PARSE_H

#include "lex.h"
#include "types.h"
#include "vector.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct ASTNode ASTNode;
typedef enum NodeKind NodeKind;

typedef struct FuncDecl FuncDecl;
typedef struct VarDecl VarDecl;

typedef struct AssignStmt AssignStmt;
typedef struct RetStmt RetStmt;
typedef struct CondStmt CondStmt;

typedef enum UnaryOp UnaryOp;
typedef enum BinaryOp BinaryOp;
typedef struct UnaryExpr UnaryExpr;
typedef struct BinaryExpr BinaryExpr;

typedef enum LiteralKind LiteralKind;
typedef struct Literal Literal;

struct FuncDecl
{
    char *name;
    Type return_type;
    ASTNode *params;
    ASTNode *body;
};

struct VarDecl 
{
    char *name;
    Type type;
    ASTNode *init;
};

struct AssignStmt 
{
    char *name;
    ASTNode *value;
};

struct RetStmt
{
    ASTNode *value;
};

struct CondStmt 
{
    ASTNode *expr;
    ASTNode *body;
};

enum UnaryOp
{
    UN_UNKNOWN = 0,
    UN_NEG     = 1,
    UN_NOT,
    UN_DEREF,
    UN_ADDR,
};

struct UnaryExpr
{
    UnaryOp un_op;
    ASTNode *expr;
};

// NOTE: The order of these matter
enum BinaryOp
{
    BIN_UNKNOWN = 0,
    BIN_ADD     = 5,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_CMP,
    BIN_CMP_NOT,
    BIN_CMP_LT,
    BIN_CMP_GT,
    BIN_CMP_LT_EQ,
    BIN_CMP_GT_EQ,
};

struct BinaryExpr
{
    BinaryOp bin_op;
    ASTNode *lhs;
    ASTNode *rhs;
};

enum LiteralKind
{
    L_INT,
    L_UINT,
    L_FLOAT,
    L_DOUBLE,
    L_CHAR,
    L_BOOL,
    L_STRING,
    L_SIZE,
};

struct Literal
{
    LiteralKind kind;
    union
    {
        intmax_t i_val;
        uintmax_t u_val;
        float f_val;
        double d_val;
        char c_val;
        bool b_val;
        struct {
            char *s_val;
            size_t s_len;
        };
        size_t size;
    };
};

void dump_literal(Literal literal);
uint8_t *copy_literal(Literal *literal, size_t *literal_size);

enum NodeKind
{
    NODE_UNKNOWN,
    NODE_NOOP,
    NODE_FUNC_DECL,
    NODE_VAR_DECL,
    NODE_RET_STMT,
    NODE_COND_STMT,
    NODE_FUNC_CALL_EXPR,
    NODE_ASSIGN_STMT,
    NODE_UNARY_EXPR,
    NODE_BINARY_EXPR,
    NODE_LITERAL_EXPR,
    NODE_REF_EXPR,
};

struct ASTNode
{
    NodeKind kind;
    ASTNode *next;
    Type type;
    union
    {
        FuncDecl func_decl;
        VarDecl var_decl;
        RetStmt ret_stmt;
        CondStmt cond_stmt;
        AssignStmt assign;
        UnaryExpr unary;
        BinaryExpr binary;
        Literal literal;
        char *ref;
    };
    bool visited;
    int line, col;
};

ASTNode *parse(Vector tokens);
void dump_ast(ASTNode *program, int level);

#endif

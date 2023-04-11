#ifndef MINI_H
#define MINI_H

#define MINI_VERSION "0.1.0"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define MAX(x, y) (x > y) ? x : y
#define MIN(x, y) (x > y) ? y : x

/* lex.c */
typedef enum {
    TOKEN_UNKNOWN = 0, // Misc
    TOKEN_EOF,
    TOKEN_CONST, // Keywords
    TOKEN_FUNC,
    TOKEN_RETURN,
    TOKEN_USE,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_INT, // Primitive Types
    TOKEN_BOOL,
    TOKEN_WALRUS, // Operators
    TOKEN_EQUAL,
    TOKEN_BANG,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LBRACE, // Symbols
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_ARROW,
    TOKEN_IDENTIFIER, // User-defined
    TOKEN_NUMBER,
} TokenKind;

extern const char *token_strings[];
const char *token_as_str(TokenKind kind);

#define IDENTIFIER_MAX_LEN  256
#define NUMBER_MAX_LEN      256
#define STRING_MAX_LEN      4096


typedef struct Token Token;
struct Token {
    TokenKind kind;
    int line, col;
    union {
        intmax_t i_val;
        uintmax_t u_val;
        float f_val;
        double d_val;
        char c_val;
        bool b_val;
        struct {
            char *data;
            size_t length;
        } str;
    };
};

typedef struct {
    Token *tokens;
    size_t pos;
    size_t size;
    size_t capacity;
} TokenStream;

TokenStream token_stream_create();
void token_stream_append(TokenStream *stream, Token token);
Token *token_stream_get(TokenStream *stream);
Token *token_stream_next(TokenStream *stream);
Token *token_stream_prev(TokenStream *stream);

TokenStream lex(FILE *file);

/* types.c */
typedef struct TypeMember TypeMember;
typedef struct Type Type;

struct TypeMember {
    TypeMember *next;
    char *name;
    Type *type;
    int align;
    int size;
    int offset;
};

typedef enum {
    TYPE_UNKNOWN,
    TYPE_VOID, // Primitive Types
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STRUCT, // User-defined
    TYPE_ENUM,
} TypeKind;

struct Type {
    TypeKind kind;
    char *name;
    int align;
    int size;
    bool is_pointer;
    union {
        TypeMember *members;
    };
};

extern const Type primitive_types[];

/* parse.c */
typedef struct Node Node;

typedef struct {
    char *name;
    Type return_type;
    Node *params; 
    Node *body;
} FuncDecl;

typedef struct {
    char *name;
    Type type;
    Node *init;
} VarDecl;

typedef struct {
    char *name;
    Node *value;
} AssignExpr;

typedef struct {
    Node *value;
} RetStmt;

typedef struct {
    char un_op;
    Node *expr;
} UnaryExpr;

typedef struct {
    char bin_op;
    Node *lhs;
    Node *rhs;
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

struct Node {
    NodeKind kind;
    Node *next;
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

Node *parse(TokenStream *stream);
void dump_ast(Node *program);

/* symbols.c */
typedef enum {
    SYMBOL_UNKNOWN,
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
} SymbolKind;

extern const char *symbol_strings[];
const char *symbol_as_str(SymbolKind kind);

typedef struct Symbol Symbol;
struct Symbol {
    SymbolKind kind;
    Symbol *next;
    Type type;
    char *name;
    int align;
    int offset;
    bool is_constant;
    bool is_initialized;
};

#define SYMBOL_TABLE_SIZE 256

typedef struct SymbolTable SymbolTable;
struct SymbolTable {
    char *name;
    Symbol *symbols[SYMBOL_TABLE_SIZE];
    SymbolTable *parent;
    SymbolTable *child;
    SymbolTable *next;
};

SymbolTable *symbol_table_create(char *table_name);
Symbol *symbol_table_insert(SymbolTable *table, char *symbol_name, SymbolKind kind);
Symbol *symbol_table_lookup(SymbolTable *table, char *symbol_name);
void symbol_table_add_child(SymbolTable *parent, SymbolTable *child);
void symbol_table_dump(SymbolTable *table);

extern SymbolTable *global_scope;
void init_global_scope();

/* optimize.c */
void optimize(Node *program);

/* ir.c */
typedef struct BasicBlock BasicBlock;
struct BasicBlock {
    int id;
    Node *first;
    Node *last;
    BasicBlock *next;
};

BasicBlock *construct_control_flow_graph(Node *root);
void print_blocks(BasicBlock *block, const char *tag);

/* codegen.c */
typedef enum {
    TARGET_LINUX_NASM_X86_64,
} TargetKind;

extern const char *target_strings[];
const char *target_as_str(TargetKind kind);

#define DEFAULT_TARGET_CODE_CAPACITY 2048

typedef struct {
    TargetKind kind;
    char *generated_code;
    size_t code_length;
    size_t code_capacity;
} TargetASM;

void target_asm_init(TargetASM *out, TargetKind kind);
void target_asm_generate_code(TargetASM *out, Node *program);
void target_asm_write_to_file(TargetASM *out, char *output_filename);
void target_asm_free(TargetASM *out);

/* util.c */
void error(const char *fmt, ...);
void error_at(int line, int col, const char *fmt, ...);
void error_at_token(Token *t, const char *fmt, ...);
void error_with_context(char *loc, const char *fmt, ...);

int str_to_int(const char *s, size_t length);
char *rand_str(size_t length);

/* main.c */
typedef struct {
    int dump_flags;
    char *input_filename;
    char *output_filename;
} MiniOptions;

#endif

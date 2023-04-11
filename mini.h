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
    TOKEN_INT, // Primitive types
    TOKEN_BOOL,
    TOKEN_WALRUS, // Operators
    TOKEN_EQUAL,
    TOKEN_BANG,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LBRACE, // symbols
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
} token_kind;

extern const char *token_strings[];
const char *token_as_str(token_kind kind);

#define IDENTIFIER_MAX_LEN  256
#define NUMBER_MAX_LEN      256
#define STRING_MAX_LEN      4096


typedef struct token token;
struct token {
    token_kind kind;
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
    token *tokens;
    size_t pos;
    size_t size;
    size_t capacity;
} token_stream;

token_stream token_stream_create();
void token_stream_append(token_stream *stream, token token);
token *token_stream_get(token_stream *stream);
token *token_stream_next(token_stream *stream);
token *token_stream_prev(token_stream *stream);

token_stream lex(FILE *file);

/* types.c */
typedef struct type_member type_member;
typedef struct type type;

struct type_member {
    type_member *next;
    char *name;
    type *type;
    int align;
    int size;
    int offset;
};

typedef enum {
    TYPE_UNKNOWN,
    TYPE_VOID, // Primitive types
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STRUCT, // User-defined
    TYPE_ENUM,
} type_kind;

struct type {
    type_kind kind;
    char *name;
    int align;
    int size;
    bool is_pointer;
    union {
        type_member *members;
    };
};

extern const type primitive_types[];

/* parse.c */
typedef struct ast_node ast_node;

typedef struct {
    char *name;
    type return_type;
    ast_node *params; 
    ast_node *body;
} func_decl;

typedef struct {
    char *name;
    type type;
    ast_node *init;
} var_decl;

typedef struct {
    char *name;
    ast_node *value;
} assign_expr;

typedef struct {
    ast_node *value;
} ret_stmt;

typedef struct {
    char un_op;
    ast_node *expr;
} unary_expr;

typedef struct {
    char bin_op;
    ast_node *lhs;
    ast_node *rhs;
} binary_expr;

typedef union {
    intmax_t i_val;
    uintmax_t u_val;
    float f_val;
    double d_val;
    char c_val;
    bool b_val;
} literal;

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
} node_kind;

struct ast_node {
    node_kind kind;
    ast_node *next;
    type type;
    union {
        func_decl func_decl;
        var_decl var_decl;
        ret_stmt ret_stmt;
        assign_expr assign;
        unary_expr unary;
        binary_expr binary;
        literal literal;
        char *ref;
    };
};

ast_node *parse(token_stream *stream);
void dump_ast(ast_node *program);

/* symbols.c */
typedef enum {
    SYMBOL_UNKNOWN,
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
} symbol_kind;

extern const char *symbol_strings[];
const char *symbol_as_str(symbol_kind kind);

typedef struct symbol symbol;
struct symbol {
    symbol_kind kind;
    symbol *next;
    type type;
    char *name;
    int align;
    int offset;
    bool is_constant;
    bool is_initialized;
};

#define SYMBOL_TABLE_SIZE 256

typedef struct symbol_table symbol_table;
struct symbol_table {
    char *name;
    symbol *symbols[SYMBOL_TABLE_SIZE];
    symbol_table *parent;
    symbol_table *child;
    symbol_table *next;
};

symbol_table *symbol_table_create(char *table_name);
symbol *symbol_table_insert(symbol_table *table, char *symbol_name, symbol_kind kind);
symbol *symbol_table_lookup(symbol_table *table, char *symbol_name);
void symbol_table_add_child(symbol_table *parent, symbol_table *child);
void symbol_table_dump(symbol_table *table);

extern symbol_table *global_scope;
void init_global_scope();

/* optimize.c */
void optimize(ast_node *program);

/* ir.c */
typedef struct basic_block basic_block;
struct basic_block {
    int id;
    ast_node *first;
    ast_node *last;
    basic_block *next;
};

basic_block *construct_control_flow_graph(ast_node *root);
void print_blocks(basic_block *block, const char *tag);

/* codegen.c */
typedef enum {
    TARGET_LINUX_NASM_X86_64,
} target_kind;

extern const char *target_strings[];
const char *target_as_str(target_kind kind);

#define DEFAULT_TARGET_CODE_CAPACITY 2048

typedef struct {
    target_kind kind;
    char *generated_code;
    size_t code_length;
    size_t code_capacity;
} target_asm;

void target_asm_init(target_asm *out, target_kind kind);
void target_asm_generate_code(target_asm *out, ast_node *program);
void target_asm_write_to_file(target_asm *out, char *output_filename);
void target_asm_free(target_asm *out);

/* util.c */
void error(const char *fmt, ...);
void error_at(int line, int col, const char *fmt, ...);
void error_at_token(token *t, const char *fmt, ...);
void error_with_context(char *loc, const char *fmt, ...);

int str_to_int(const char *s, size_t length);
char *rand_str(size_t length);

/* main.c */
typedef struct {
    int dump_flags;
    char *input_filename;
    char *output_filename;
} mini_opts;

#endif

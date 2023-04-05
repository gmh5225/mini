/* mini compiler */

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MINI_VERSION "0.1.0"

/* Helper functions */
void fail(const char *fmt, ...);
char *read_whole_file(char *filepath);
int str_to_int(const char *s, size_t length);

/* Type definitions */
typedef enum {
    /* Misc */
    TOKEN_UNKNOWN,
    TOKEN_EOF,
    /* Keywords */
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_FUNC,
    TOKEN_GOTO,
    TOKEN_USE,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    /* Primitive Types */
    TOKEN_INT,
    TOKEN_BOOL,
    /* Operators */
    TOKEN_EQUAL,
    TOKEN_BANG,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    /* Symbols */
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOUBLE_COLON,
    TOKEN_DOT,
    TOKEN_COMMA,
    /* User-Defined */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
} token_type;

typedef struct {
    token_type type;
    const char *lexeme;
    size_t length;
    int line, col;
} token;

const char *token_strings[] = {
    [TOKEN_UNKNOWN] = "[UNKNOWN]",
    [TOKEN_EOF] = "[EOF]",
    [TOKEN_LET] = "let",
    [TOKEN_CONST] = "const",
    [TOKEN_FUNC] = "func",
    [TOKEN_GOTO] = "goto",
    [TOKEN_USE] = "use",
    [TOKEN_STRUCT] = "struct",
    [TOKEN_ENUM] = "enum",
    [TOKEN_IF] = "if",
    [TOKEN_ELIF] = "elif",
    [TOKEN_ELSE] = "else",
    [TOKEN_TRUE] = "true",
    [TOKEN_FALSE] = "false",
    [TOKEN_INT] = "int",
    [TOKEN_BOOL] = "bool",
    [TOKEN_EQUAL] = "=",
    [TOKEN_PLUS] = "+",
    [TOKEN_MINUS] = "-",
    [TOKEN_STAR] = "*",
    [TOKEN_SLASH] = "/",
    [TOKEN_LBRACE] = "{",
    [TOKEN_RBRACE] = "}",
    [TOKEN_LPAREN] = "(",
    [TOKEN_RPAREN] = ")",
    [TOKEN_LBRACKET] = "[",
    [TOKEN_RBRACKET] = "]",
    [TOKEN_SEMICOLON] = ";",
    [TOKEN_COLON] = ":",
    [TOKEN_DOUBLE_COLON] = "::",
    [TOKEN_DOT] = ".",
    [TOKEN_COMMA] = ",",
    [TOKEN_IDENTIFIER] = "[IDENTIFIER]",
    [TOKEN_NUMBER] = "[NUMBER]",
};

#define TOKSTR(type) token_strings[type]

typedef enum {
    UNARY_BITWISE_NOT,      // ~
    UNARY_LOGICAL_NOT,      // !
    UNARY_NEGATE,           // -
    UNARY_DEREFERENCE,      // *
    UNARY_ADDRESS,          // &
} unary_op;

const char *unary_op_strings[] = {
    [UNARY_BITWISE_NOT] = "~",
    [UNARY_LOGICAL_NOT] = "!",
    [UNARY_NEGATE] = "-",
    [UNARY_DEREFERENCE] = "*",
    [UNARY_ADDRESS] = "&",
};

#define UNOPSTR(type) unary_op_strings[type]

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

#define BINOPSTR(type) binary_op_strings[type]

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

#define TYPESTR(type) type_strings[type]

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
    ast_node *assignment;
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
        unary_op unary;
        binary_op binary;
        variable *variable;
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
    NODE_LITERAL,
} ast_node_type;

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

#define NODESTR(type) node_strings[type]

typedef struct ast_node {
    ast_node_type type;
    union {
        variable variable;
        function function;
        expression expression;
    } as;
    struct ast_node **children;
    size_t num_children;
    size_t children_capacity;
} ast_node;

#define AS_VAR(node) ((*node).as.variable)
#define AS_FUNC(node) ((*node).as.function)
#define AS_EXPR(node) ((*node).as.expression)

#define SYMBOL_TABLE_SIZE 4096

typedef struct symbol_info {
    char *name;
    ast_node_type type;
    int scope_level;
    ast_node *value;
    bool is_constant;
    bool is_initialized;
    struct symbol_info *next;
} symbol_info;

typedef struct symbol_table {
    symbol_info *symbols[SYMBOL_TABLE_SIZE];
    int scope_level;
} symbol_table;

symbol_info *add_symbol(symbol_table *table, char *name, ast_node_type type);
symbol_info *lookup_symbol(symbol_table *table, char *name);
void enter_scope(symbol_table *table);
void exit_scope(symbol_table *table);
void dump_symbols(symbol_table *table);

char *source_code = NULL;
token current_token = {0};
token previous_token = {0};
ast_node *ast_root = NULL;
symbol_table symbols = {0};
int line_no = 1, col_no = 1;

token next_token();
void advance();
bool check(token_type expected);
bool match(token_type expected);

ast_node *parse_program();
ast_node *parse_statement();
ast_node *parse_block();
ast_node *parse_variable();
ast_node *parse_function();
ast_node *parse_function_call();
ast_node *parse_conditional();
ast_node *parse_literal();
ast_node *parse_expression();

int main(int argc, char **argv) {
    char *input_filename = NULL;
    char *output_filename = "a.out";
    uint8_t optimization_level = 0;

    // Parse command line arguments
    bool skip = false;
    for (int i = 1; i < argc; i++) {
        if (skip) { skip = false; continue; }
        char *arg = argv[i];
        if (strcmp(arg, "-o") == 0) {
            if (i + 1 >= argc) {
                fail("not enough arguments for option '%s'", arg);
            }
            output_filename = argv[i + 1];
            skip = true;
        } else if (strcmp(arg, "-O") == 0) {
            if (i + 1 >= argc) {
                fail("not enough arguments for option '%s'", arg);
            }
            char *value = argv[i + 1];
            optimization_level = (uint8_t)str_to_int(value, strlen(value));
            skip = true;
        } else {
            input_filename = arg;
        }
    }
    if (!input_filename) {
        fail("input file is required");
    }

    source_code = read_whole_file(input_filename);
    ast_root = parse_program();

    //for (size_t i = 0; i < ast_root->num_children; i++) {
    //    ast_node *child = ast_root->children[i];
    //    printf("node id (%d)\n", child->type);
    //}

    dump_symbols(&symbols);

    printf("compiling to artifact '%s' with optimization level %d\n", 
            output_filename, optimization_level);

    if (source_code) {
        free(source_code);
    }
    return 0;
}

/* Lexer */
bool is_whitespace(char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }
bool is_alphabetic(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'; }
bool is_numeric(char c) { return c >= '0' && c <= '9'; }
bool is_alphanumeric(char c) { return is_alphabetic(c) || is_numeric(c); }

token make_token(token_type type) {
    return (token){
        .type = type, 
        .lexeme = NULL, 
        .length = 0,
        .line = line_no,
        .col = col_no,
    };
}

token next_token(char *input) {
    static char *p = NULL;
    static char *start = NULL;

    if (!p) {
        p = input;
        start = input;
    }

    // Skip whitespace
    while (is_whitespace(*p)) {
        if (*p == '\n') { 
            line_no++; col_no = 1; 
        } else {
            col_no++;
        }
        p++;
    }

    // Check for end of source
    if (*p == '\0') { return make_token(TOKEN_EOF); }

    // Skip comments
    if (*p == '#') {
        while (*p != '\n') { p++; }
        line_no++;
        col_no = 1;
        start = p;
        return next_token(input);
    }

    if (is_alphabetic(*p)) {
        start = p;
        while (is_alphanumeric(*p)) { p++; col_no++; }
        size_t length = (size_t)(p - start);

        for (token_type type = TOKEN_LET; type <= TOKEN_FALSE; type++) {
            if (memcmp(start, TOKSTR(type), length) == 0) {
                token keyword = make_token(type);
                start = p;
                return keyword;
            }
        }

        token identifier = make_token(TOKEN_IDENTIFIER);
        identifier.lexeme = start; identifier.length = length;
        start = p;
        return identifier;
    } else if (is_numeric(*p)) {
        start = p;
        while (is_numeric(*p)) { p++; col_no++; }
        size_t length = (size_t)(p - start);

        token number = make_token(TOKEN_NUMBER);
        number.lexeme = start; number.length = length;
        start = p;
        return number;
    }

    token symbol;
    switch (*p) {
        case '+': symbol = make_token(TOKEN_PLUS); break;
        case '-': symbol = make_token(TOKEN_MINUS); break;
        case '*': symbol = make_token(TOKEN_STAR); break;
        case '/': symbol = make_token(TOKEN_SLASH); break;
        case '=': symbol = make_token(TOKEN_EQUAL); break;
        case '!': symbol = make_token(TOKEN_BANG); break;
        case ';': symbol = make_token(TOKEN_SEMICOLON); break;
        case '{': symbol = make_token(TOKEN_LBRACE); break;
        case '}': symbol = make_token(TOKEN_RBRACE); break;
        case '(': symbol = make_token(TOKEN_LPAREN); break;
        case ')': symbol = make_token(TOKEN_RPAREN); break;
        case '[': symbol = make_token(TOKEN_LBRACKET); break;
        case ']': symbol = make_token(TOKEN_RBRACKET); break;
        default: fail("unknown symbol at line %zu, col %zu: %c", line_no, col_no, *p);
    }

    p++; col_no++;
    start = p;
    return symbol;
}

/* Parser */

void advance() {
    previous_token = current_token;
    current_token = next_token(source_code);
}

bool check(token_type expected) {
    if (current_token.type != expected) return false;
    return true;
}

bool match(token_type expected) {
    if (!check(expected)) return false;
    advance();
    return true;
}

void expect(token_type expected) {
    if (current_token.type != expected) {
        fail("at line %d, col %d: expected %s, got %s",
                current_token.line, current_token.col,
                TOKSTR(expected), TOKSTR(current_token.type));
    }
    advance();
}

void unexpected(const char *location) {
    fail("unexpected token '%s' while parsing %s at line %d, col %d", 
        TOKSTR(current_token.type), location, current_token.line, current_token.col);
}

void copy_previous(char **buf) {
    *buf = calloc(previous_token.length + 1, sizeof(char));
    memcpy(*buf, previous_token.lexeme, previous_token.length);
}

ast_node *make_ast_node(ast_node_type type) {
    ast_node *node = malloc(sizeof(ast_node));
    node->type = type;
    node->children_capacity = AST_CHILDREN_CAPACITY;
    node->children = calloc(node->children_capacity, sizeof(ast_node *));
    node->num_children = 0;
    return node;
}

size_t add_ast_child(ast_node *parent, ast_node *child) {
    if (parent->num_children >= parent->children_capacity) {
        parent->children_capacity <<= 1;
        void *tmp = realloc(parent->children, sizeof(ast_node *) * parent->children_capacity);
        parent->children = tmp;
    }
    parent->children[parent->num_children++] = child;
    return parent->num_children - 1;
}

type_kind resolve_type(const char *identifier, size_t length) {
    for (type_kind kind = TYPE_BOOL; kind < TYPE_CHAR; kind++) {
        if (memcmp(identifier, type_strings[kind], length) == 0) {
            return kind;
        }
    }
    return TYPE_CUSTOM;
}

type_info parse_type() {
    type_info type = {0};
    expect(TOKEN_IDENTIFIER);
    type.kind = resolve_type(previous_token.lexeme, previous_token.length);
    copy_previous(&type.name);
    return type;
}

void print_exprs(ast_node *root) {
    if (!root) return;

    switch (AS_EXPR(root).type) {
        case EXPR_BOOL:
            printf("BOOL(%d)", AS_EXPR(root).as.boolean);
            break;
        case EXPR_INT:
            printf("INT(%d)", AS_EXPR(root).as.integer);
            break;
        case EXPR_UNARY:
            printf("EXPR_UNARY(%s)", UNOPSTR(AS_EXPR(root).as.unary));
            break;
        case EXPR_BINARY:
            printf("EXPR_BINARY(%s)", BINOPSTR(AS_EXPR(root).as.binary));
            break;
        default: 
            printf("[UNKNOWN EXPR TYPE]");
    }

    printf(" (%ld children)\n", root->num_children);

    for (size_t i = 0; i < root->num_children; i++) {
        print_exprs(root->children[i]);
    }
}

void parse_factor(ast_node *exprs) {
    ast_node *factor = make_ast_node(NODE_EXPRESSION);

    if (match(TOKEN_IDENTIFIER)) {
        char *variable_name = NULL;
        copy_previous(&variable_name);
        symbol_info *variable_symbol = lookup_symbol(&symbols, variable_name);
        if (!variable_symbol) {
            fail("symbol '%s' is not declared", variable_name);
        }
        free(variable_name);
    } else if (match(TOKEN_NUMBER)) {
        AS_EXPR(factor).type = EXPR_INT;
        AS_EXPR(factor).as.integer = str_to_int(previous_token.lexeme, previous_token.length);
    } else if (match(TOKEN_TRUE)) {
        AS_EXPR(factor).type = EXPR_BOOL;
        AS_EXPR(factor).as.boolean = true;
    } else if (match(TOKEN_FALSE)) {
        AS_EXPR(factor).type = EXPR_BOOL;
        AS_EXPR(factor).as.boolean = false;
    } else {
        unexpected("expression");
    }

    add_ast_child(exprs, factor);
}

void parse_term(ast_node *exprs) {
    parse_factor(exprs);
    for (;;) {
        binary_op b_op;
        if (match(TOKEN_STAR)) { b_op = BINARY_MULTIPLY; }
        else if (match(TOKEN_SLASH)) { b_op = BINARY_DIVIDE; }
        else { break; }

        parse_term(exprs);

        ast_node *node = make_ast_node(NODE_EXPRESSION);
        AS_EXPR(node).type = EXPR_BINARY;
        AS_EXPR(node).as.binary = b_op;
        add_ast_child(exprs, node);
    }
}

ast_node *parse_expression_internal(ast_node *exprs) {
    unary_op u_op;
    bool has_unary_expr = true;
    if (match(TOKEN_MINUS)) { u_op = UNARY_NEGATE; } 
    else if (match(TOKEN_BANG)) { u_op = UNARY_LOGICAL_NOT; }
    else if (match(TOKEN_STAR)) { u_op = UNARY_DEREFERENCE; }
    else { has_unary_expr = false; }

    parse_term(exprs);

    if (has_unary_expr) {
        ast_node *node = make_ast_node(NODE_EXPRESSION);
        AS_EXPR(node).type = EXPR_UNARY;
        AS_EXPR(node).as.unary = u_op;
        add_ast_child(exprs, node);
    }

    for (;;) {
        binary_op b_op;
        if (match(TOKEN_PLUS)) { b_op = BINARY_ADD; }
        else if (match(TOKEN_MINUS)) { b_op = BINARY_SUBTRACT; }
        else { break; }

        parse_term(exprs);

        ast_node *node = make_ast_node(NODE_EXPRESSION);
        AS_EXPR(node).type = EXPR_BINARY;
        AS_EXPR(node).as.binary = b_op;
        add_ast_child(exprs, node);
    }

    return NULL;
}

ast_node *parse_expression() {
    ast_node *exprs = make_ast_node(NODE_EXPRESSION);
    exprs->as.expression.type = -1;
    parse_expression_internal(exprs);
    return exprs;
}

ast_node *parse_program() {
    ast_node *program = make_ast_node(NODE_PROGRAM);

    advance();
    while (!match(TOKEN_EOF)) {
#ifdef DEBUG
        printf("%s", TOKSTR(current_token.type));
        if (current_token.length > 0) {
            printf(":%.*s", (int)current_token.length, current_token.lexeme);
        }
        printf("\n");
#endif
        ast_node *statement = parse_statement();
        add_ast_child(program, statement);
    }

    return program;
}

ast_node *parse_statement() {
    ast_node *node = NULL;
    if (match(TOKEN_LET) || match(TOKEN_CONST)) {
        node = parse_variable();
        expect(TOKEN_SEMICOLON);
    } else if (match(TOKEN_FUNC)) {
        node = parse_function();
    } else {
        unexpected("statement");
    }
    return node;
}

ast_node *parse_variable() {
    bool is_constant = (previous_token.type == TOKEN_CONST) ? true : false;

    // Parse identifier
    expect(TOKEN_IDENTIFIER);
    char *name = NULL;
    copy_previous(&name);

    symbol_info *variable_symbol = add_symbol(&symbols, name, NODE_VARIABLE);
    AS_VAR(variable_symbol->value).type.kind = TYPE_UNKNOWN;
    variable_symbol->is_constant = is_constant;
    variable_symbol->is_initialized = false;

    // Parse initial expression here and infer type from this expression,
    // or just parse type (uninitialized)
    if (match(TOKEN_EQUAL)) {
        ast_node *assignment = parse_expression();
        print_exprs(assignment);
        AS_VAR(variable_symbol->value).assignment = assignment;
        variable_symbol->is_initialized = true;
    } else {
        expect(TOKEN_COLON);
    }

    return variable_symbol->value;
}

function_param *make_function_param(char *name, type_info type) {
    function_param *param = malloc(sizeof(function_param));
    param->name = name;
    param->type = type;
    return param;
}

ast_node *parse_function() {
    ast_node *node = make_ast_node(NODE_FUNCTION);

    // Parse identifier
    expect(TOKEN_IDENTIFIER);
    char *name = NULL;
    copy_previous(&name);

    symbol_info *function_symbol = add_symbol(&symbols, name, NODE_FUNCTION);
    function_symbol->is_initialized = true;

    // Parse function parameters
    expect(TOKEN_LPAREN);

    function_param *params = NULL;
    for (;;) {
        if (match(TOKEN_RPAREN)) {
            break;
        } else if (match(TOKEN_COMMA)) {
            continue;
        } else if (match(TOKEN_IDENTIFIER)) {
            char *name = NULL;
            copy_previous(&name);

            expect(TOKEN_COLON);
            type_info type = parse_type();

            function_param *param = make_function_param(name, type);
            if (!params) {
                params = param;
            } else {
                function_param *iter = params;
                while (iter) { iter = iter->next; }
                iter = param;
            }
        } else {
            unexpected("function parameters");
        }
    }
    node->as.function.parameters = params;

    // Parse return type
    if (check(TOKEN_IDENTIFIER)) {
        node->as.function.return_type = parse_type();
    } else {
        node->as.function.return_type.kind = TYPE_VOID;
    }

    // Parse function body
    ast_node *body = parse_block();
    add_ast_child(node, body);

    return node;
}

ast_node *parse_block() {
    enter_scope(&symbols);
    ast_node *node = make_ast_node(NODE_BLOCK);

    expect(TOKEN_LBRACE);

    for (;;) {
        if (match(TOKEN_RBRACE)) {
            break;
        }

        ast_node *statement = parse_statement();
        add_ast_child(node, statement);
    }

    dump_symbols(&symbols);
    exit_scope(&symbols);
    return node;
}

/* Symbol Table */
uint64_t hash(char *s) {
    uint64_t hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

symbol_info *add_symbol(symbol_table *table, char *name, ast_node_type type) {
    uint64_t index = hash(name) % SYMBOL_TABLE_SIZE;
    symbol_info *info = malloc(sizeof(symbol_info));

    info->name = name;
    info->value = make_ast_node(type);
    info->type = type;
    info->scope_level = table->scope_level;

    symbol_info *exists = table->symbols[index];
    if (exists) {
        if (memcmp(exists->name, name, strlen(name)) == 0) {
            fail("symbol '%s' already exists in scope", exists->name);
        }
        info->next = exists;
    }

    table->symbols[index] = info;
    return table->symbols[index];
}

symbol_info *lookup_symbol(symbol_table *table, char *name) {
    uint64_t index = hash(name) % SYMBOL_TABLE_SIZE;
    symbol_info *info = table->symbols[index];

    while (info) {
        if (memcmp(info->name, name, strlen(name)) == 0) {
            return info;
        }
        info = info->next;
    }

    return NULL;
}

void enter_scope(symbol_table *table) {
    table->scope_level++;
}

void exit_scope(symbol_table *table) {
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        symbol_info *info = table->symbols[i];
        symbol_info *prev = NULL;

        while (info) {
            if (info->scope_level == table->scope_level) {
                if (prev == NULL) {
                    table->symbols[i] = info->next;
                } else {
                    prev->next = info->next;
                }
                free(info);
            } else {
                prev = info;
            }

            info = info->next;
        }
    }

    table->scope_level--;
}

void dump_symbols(symbol_table *table) {
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        symbol_info *info = table->symbols[i];
        while (info) {
            printf("[%d] : %s : %s", 
                    info->scope_level,
                    NODESTR(info->value->type),
                    info->name);

            if (info->next) {
                printf("  ->  ");
            } else {
                printf("\n");
            }

            info = info->next;
        }
    }
}

/* Misc */
int str_to_int(const char *s, size_t length) {
    int n = 0;
    for (size_t i = 0; i < length; i++) {
        n = n * 10 + (s[i] - '0');
    }
    return n;
}

char *read_whole_file(char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fail("couldn't open file '%s'", filepath);
    }

    fseek(f, 0L, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);

    char *buf = malloc(sizeof(char) * (file_size + 1));
    if (!buf) {
        fail("couldn't malloc buffer to read source code");
    }

    size_t nread = fread(buf, sizeof(char), file_size, f);
    if (nread != file_size) {
        fail("only read %zu bytes from file '%s' with size %zu", nread, filepath, file_size);
    }
    buf[nread] = 0; // This null terminator is really important...
    fclose(f);

    return buf;
}

void fail(const char *fmt, ...) {
    fprintf(stderr, "mini: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

#include "parser.h"
#include "lexer.h"
#include "symbol_table.h"
#include "util.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

symbol_table *global_scope = NULL;
static char *source_code = NULL;
static token current_token = {0};
static token previous_token = {0};
static symbol_table *previous_table = NULL;
static symbol_table *current_table = NULL;

ast_node *parse_program();
ast_node *parse_declaration();
ast_node *parse_statement();
ast_node *parse_variable();
ast_node *parse_function();
ast_node *parse_function_call();
ast_node *parse_conditional();
ast_node *parse_literal();
ast_node *parse_expression();

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
                token_as_str(expected), token_as_str(current_token.type));
    }
    advance();
}

void unexpected(const char *location) {
    fail("unexpected token '%s' while parsing %s at line %d, col %d", 
            token_as_str(current_token.type), location, current_token.line, current_token.col);
}

void copy_previous(char **buf) {
    *buf = calloc(previous_token.length + 1, sizeof(char));
    memcpy(*buf, previous_token.lexeme, previous_token.length);
}

ast_node *make_ast_node(ast_node_type type) {
    ast_node *node = malloc(sizeof(ast_node));
    node->type = type;
    node->child_capacity = AST_CHILDREN_CAPACITY;
    node->children = calloc(node->child_capacity, sizeof(ast_node *));
    node->num_children = 0;
    return node;
}

size_t add_ast_child(ast_node *parent, ast_node *child) {
    if (parent->num_children >= parent->child_capacity) {
        parent->child_capacity <<= 1;
        void *tmp = realloc(parent->children, sizeof(ast_node *) * parent->child_capacity);
        parent->children = tmp;
    }
    parent->children[parent->num_children++] = child;
    return parent->num_children - 1;
}

void enter_scope(symbol_table *next_table) {
    previous_table = current_table;
    current_table = next_table;
}

void exit_scope() {
    current_table = previous_table;
}

symbol_table *create_scope(char *scope_name) {
    symbol_table *table = symbol_table_create(scope_name);
    symbol_table_add_child(current_table, table);
    return table;
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

void print_exprs_impl(ast_node *root, int level) {
    for (int i = 0; i < level; i++) {
        printf("    ");
    }

    switch (AS_EXPR(root).type) {
        case EXPR_BOOL:
            printf("BOOL(%s)", AS_EXPR(root).as.boolean ? "true" : "false");
            break;
        case EXPR_INT:
            printf("INT(%d)", AS_EXPR(root).as.integer);
            break;
        case EXPR_UNARY:
            printf("EXPR_UNARY(%s)", unary_as_str(AS_EXPR(root).as.unary));
            break;
        case EXPR_BINARY:
            printf("EXPR_BINARY(%s)", binary_as_str(AS_EXPR(root).as.binary));
            break;
        default: 
            printf("[UNKNOWN EXPR TYPE]");
    }

    printf(" (%ld children)\n", root->num_children);

    for (size_t i = 0; i < root->num_children; i++) {
        print_exprs_impl(root->children[i], level + 1);
    }
}

void print_exprs(ast_node *root) {
    if (!root) return;
    print_exprs_impl(root, 0);
}

void parse_factor(ast_node *exprs) {
    ast_node *factor = make_ast_node(NODE_EXPRESSION);

    if (match(TOKEN_IDENTIFIER)) {
        char *var_name = NULL;
        copy_previous(&var_name);
        symbol_info *symbol = symbol_table_lookup(current_table, var_name);
        if (!symbol) {
            fail("symbol '%s' is not declared", var_name);
        }
        free(var_name);
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

ast_node *parse_expression_impl(ast_node *exprs) {
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
    parse_expression_impl(exprs);
    return exprs;
}


ast_node *parse_declaration() {
    ast_node *decl = NULL;
    if (match(TOKEN_IDENTIFIER) || match(TOKEN_CONST)) {
        decl = parse_variable();
        expect(TOKEN_SEMICOLON);
    } else if (match(TOKEN_FUNC)) {
        decl = parse_function();
    } else {
        unexpected("toplevel statement");
    }
    return decl;
}

ast_node *parse_statement() {
    ast_node *stmt = NULL;
    if (match(TOKEN_IDENTIFIER) || match(TOKEN_CONST)) {
        stmt = parse_variable();
        expect(TOKEN_SEMICOLON);
    } else {
        unexpected("block statement");
    }
    return stmt;
}

ast_node *parse_variable() {
    bool is_constant = false;
    if (previous_token.type == TOKEN_CONST) {
        is_constant = true;
        expect(TOKEN_IDENTIFIER);
    }

    // Parse identifier
    char *var_name = NULL;
    copy_previous(&var_name);

    ast_node *var = make_ast_node(NODE_VARIABLE);
    AS_VAR(var).name = var_name;
    AS_VAR(var).type.kind = TYPE_UNKNOWN;

    symbol_info *symbol = symbol_table_insert(current_table, var_name, NODE_VARIABLE);
    if (!symbol) {
        fail("symbol '%s' already exists in scope", var_name);
    }
    symbol->is_constant = is_constant;
    symbol->is_initialized = false;

    // Parse initial expression here and infer type from this expression,
    // or just parse type (uninitialized)
    if (match(TOKEN_WALRUS)) {
        ast_node *assignment = parse_expression();
        print_exprs(assignment);
        AS_VAR(var).assignment = assignment;
        symbol->is_initialized = true;
    } else {
        expect(TOKEN_COLON);
        AS_VAR(var).type = parse_type();

        if (match(TOKEN_EQUAL)) {
            ast_node *assignment = parse_expression();
            print_exprs(assignment);
            AS_VAR(var).assignment = assignment;
            symbol->is_initialized = true;
        }
    }

    return var;
}

function_param *make_function_param(char *name, type_info type) {
    function_param *param = malloc(sizeof(function_param));
    param->name = name;
    param->type = type;
    return param;
}

ast_node *parse_function() {
    // Parse identifier
    expect(TOKEN_IDENTIFIER);
    char *func_name = NULL;
    copy_previous(&func_name);

    ast_node *func = make_ast_node(NODE_FUNCTION);
    AS_FUNC(func).name = func_name;
    AS_FUNC(func).return_type.kind = TYPE_VOID;

    symbol_table *func_scope = create_scope(func_name);
    symbol_info *symbol = symbol_table_insert(current_table, func_name, NODE_FUNCTION);
    symbol->is_initialized = true;

    enter_scope(func_scope);

    // Add the function symbol in its own scope to allow for recursion
    symbol_table_insert(current_table, func_name, NODE_FUNCTION);

    // Parse function parameters
    expect(TOKEN_LPAREN);

    function_param *params = NULL;
    for (;;) {
        if (match(TOKEN_RPAREN)) { break; } 
        else if (match(TOKEN_COMMA)) { continue; } 
        else if (match(TOKEN_IDENTIFIER)) {
            char *param_name = NULL;
            copy_previous(&param_name);

            expect(TOKEN_COLON);
            type_info type = parse_type();

            function_param *param = make_function_param(param_name, type);
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
    AS_FUNC(func).parameters = params;

    // Parse return type
    if (check(TOKEN_IDENTIFIER)) {
        AS_FUNC(func).return_type = parse_type();
    }

    // Parse function body
    expect(TOKEN_LBRACE);

    for (;;) {
        if (match(TOKEN_RBRACE)) { break; }
        ast_node *statement = parse_statement();
        add_ast_child(func, statement);
    }

    exit_scope();

    return func;
}

ast_node *parse_program(char *source) {
    source_code = source;
    current_table = previous_table = global_scope;
    ast_node *program = make_ast_node(NODE_PROGRAM);

    advance();
    while (!match(TOKEN_EOF)) {
        ast_node *statement = parse_declaration();
        add_ast_child(program, statement);
    }

    return program;
}


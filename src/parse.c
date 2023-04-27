#include "parse.h"
#include "compile.h"
#include "symbols.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SymbolTable *current_scope;  // The current scope of the parser
static Vector stream;               // The stream of tokens to parse
static size_t stream_pos;           // The position in the token stream
static ASTNode *stack_top;          // The top of the expression stack

void enter_scope(SymbolTable *new_scope) { current_scope = new_scope; }
void exit_scope() { current_scope = current_scope->parent; }
static Token *tok() { return (Token *)vector_get(&stream, stream_pos); }
static Token *consume()
{
    Token *token = tok();
    stream_pos++;
    return token;
}

static bool match(TokenKind want)
{
    bool matches = tok()->kind == want;
    if (matches) consume();
    return matches;
}

static Token *expect(TokenKind expected)
{
    Token *got = consume();
    if (got->kind != expected) {
        fatal("at line %d, col %d: expected `%s`, got `%s`",
            got->line, got->col, token_as_str(expected), token_as_str(got->kind));
    }
    return got;
}

static ASTNode *make_node(NodeKind kind)
{
    ASTNode *node = calloc(1, sizeof(struct ASTNode));
    node->kind = kind;
    node->visited = false;
    node->type = primitive_types[TYPE_VOID];
    return node;
}

void push_expr_node(ASTNode *expr)
{
    if (!stack_top) {
        stack_top = expr;
    }
    else {
        ASTNode *temp = stack_top;
        stack_top = expr;
        stack_top->next = temp;
    }
}

ASTNode *pop_expr_node()
{
    ASTNode *ret = stack_top;
    stack_top = stack_top->next;
    // Propagate inner-most expression type to outer expressions
    if (stack_top) {
        stack_top->type = ret->type;
    }
    return ret;
}

static void parse_factor();
static void parse_term();
static ASTNode *parse_block(bool);

static ASTNode *parse_unary_expr(char un_op, int line, int col)
{
    ASTNode *node = make_node(NODE_UNARY_EXPR);
    node->line = line; node->col = col;
    node->unary.un_op = un_op;
    node->unary.expr = pop_expr_node();
    node->type = node->unary.expr->type;
    return node;
}

static ASTNode *parse_binary_expr(char bin_op, int line, int col)
{
    ASTNode *node = make_node(NODE_BINARY_EXPR);
    node->line = line; node->col = col;
    node->binary.bin_op = bin_op;

    node->binary.lhs = pop_expr_node();
    parse_term();
    node->binary.rhs = pop_expr_node();

    Type lhs_type = node->binary.lhs->type;
    Type rhs_type = node->binary.rhs->type;
    if (!types_equal(lhs_type, rhs_type)) {
        fatal("at line %d, col %d: type mismatch in binary expression\n"
            "LHS(%s) != RHS(%s)", line, col, lhs_type.name, rhs_type.name);
    }

    node->type = node->binary.lhs->type;
    return node;
}

static void parse_factor()
{
    ASTNode *node = make_node(NODE_LITERAL_EXPR);

    Token *token = consume();
    switch (token->kind) {
        case TOKEN_IDENTIFIER:
            // Check to see if the variable we are referencing is valid
            char *var_name = token->str.data;
            Symbol *var_sym = symbol_table_lookup(current_scope, var_name);
            if (!var_sym)
                fatal("at line %d, col %d: unknown Symbol `%s`", 
                        token->line, token->col, var_name);
            node->kind = NODE_REF_EXPR;
            node->type = var_sym->type;
            node->ref = var_name;
            break;
        case TOKEN_NUMBER:
            // TODO: Infer type from number here.
            // For now, we just assume its an `int`
            node->type = primitive_types[TYPE_INT];
            node->literal.kind = L_INT;
            node->literal.i_val = token->i_val;
            break;
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            node->type = primitive_types[TYPE_BOOL];
            node->literal.kind = L_BOOL;
            node->literal.b_val = token->b_val;
            break;
        default:
            fatal("at line %d, col %d: invalid Token `%s` while parsing expression",
                    token->line, token->col, token_as_str(token->kind));
    }

    push_expr_node(node);
}

void parse_term()
{
    parse_factor();
    for (;;) {
        int line = tok()->line;
        int col = tok()->col;
        BinaryOp bin_op = BIN_UNKNOWN;
        switch (tok()->kind) {
            case TOKEN_STAR: 
                bin_op = BIN_MUL; consume(); 
                break;
            case TOKEN_SLASH: 
                bin_op = BIN_DIV; consume(); 
                break;
            default: break;
        }

        if (bin_op == BIN_UNKNOWN) break;
        push_expr_node(parse_binary_expr(bin_op, line, col));
    }
}

static ASTNode *parse_expression()
{
    int line = tok()->line;
    int col = tok()->col;
    UnaryOp un_op = UN_UNKNOWN;
    switch (tok()->kind) {
        case TOKEN_MINUS: 
            un_op = UN_NEG; consume(); 
            break;
        case TOKEN_BANG: 
            un_op = UN_NOT; consume(); 
            break;
        case TOKEN_STAR: 
            un_op = UN_DEREF; consume(); 
            break;
        default: break;
    }

    parse_term();

    if (un_op != UN_UNKNOWN) {
        push_expr_node(parse_unary_expr(un_op, line, col));
    }

    for (;;) {
        line = tok()->line;
        col = tok()->col;
        BinaryOp bin_op = BIN_UNKNOWN;
        switch (tok()->kind) {
            case TOKEN_PLUS: 
                bin_op = BIN_ADD; consume(); 
                break;
            case TOKEN_MINUS: 
                bin_op = BIN_SUB; consume(); 
                break;
            case TOKEN_DOUBLE_EQUAL: 
                bin_op = BIN_CMP; consume(); 
                break;
            case TOKEN_NOT_EQUAL: 
                bin_op = BIN_CMP_NOT; consume(); 
                break;
            case TOKEN_LANGLE: 
                bin_op = BIN_CMP_LT; consume(); 
                break;
            case TOKEN_RANGLE: 
                bin_op = BIN_CMP_GT; consume(); 
                break;
            case TOKEN_LESS_THAN_EQUAL: 
                bin_op = BIN_CMP_LT_EQ; consume(); 
                break;
            case TOKEN_GREATER_THAN_EQUAL: 
                bin_op = BIN_CMP_GT_EQ; consume(); 
                break;
            default: break;
        }

        if (bin_op == BIN_UNKNOWN) break;
        push_expr_node(parse_binary_expr(bin_op, line, col));
    }

    return pop_expr_node();
}

static ASTNode *parse_conditional()
{
    Token *conditional = consume();

    ASTNode *node = make_node(NODE_COND_STMT);

    switch (conditional->kind) {
        case TOKEN_IF:
        case TOKEN_ELIF:
            // TODO: add typechecking to see if expression is a logical expression
            node->cond_stmt.expr = parse_expression();
            break;
        case TOKEN_ELSE:
            node->cond_stmt.expr = NULL;
            break;
        default: fatal("at line %d, col %d: invalid conditional",
                         conditional->line, conditional->col);
    }

    node->cond_stmt.body = parse_block(false);
    return node;
}

static Type parse_type()
{
    Token *token = expect(TOKEN_IDENTIFIER);
    char *type_name = token->str.data;

    // Search for type Symbol in current scope
    Symbol *type_sym = symbol_table_lookup(current_scope, type_name);
    if (!type_sym) 
        fatal("at line %d, col %d: unknown type `%s`",
                token->line, token->col, type_name);

    return type_sym->type;
}

static ASTNode *parse_variable_declaration(char *var_name)
{
    int line = tok()->line;
    int col = tok()->col;
    // Check if the variable is a constant and parse identifier if not yet parsed
    bool is_constant = false;
    if (!var_name) {
        expect(TOKEN_CONST);
        is_constant = true;
        var_name = expect(TOKEN_IDENTIFIER)->str.data;
    }

    ASTNode *node = make_node(NODE_VAR_DECL);
    node->line = line; node->col = col;
    node->var_decl.name = var_name;
    node->var_decl.type = primitive_types[TYPE_VOID];

    // Insert variable into current scope
    Symbol *var_sym = symbol_table_insert(current_scope, var_name, SYMBOL_VARIABLE);
    if (!var_sym) {
        fatal("at line %d, col %d: variable `%s` redeclared in scope", 
                line, col, var_name);
    }
    var_sym->is_constant = is_constant;

    // Parse assignment and/or type declaration of variable
    if (match(TOKEN_WALRUS)) {
        node->var_decl.init = parse_expression();
        // Infer type from expression
        node->var_decl.type = node->var_decl.init->type; 
        var_sym->type = node->var_decl.init->type;
        var_sym->is_initialized = true;
    }
    else {
        expect(TOKEN_COLON);
        node->var_decl.type = parse_type();

        if (match(TOKEN_EQUAL)) {
            node->var_decl.init = parse_expression();

            Type decl_type = node->var_decl.type;
            Type assign_type = node->var_decl.init->type;
            if (!types_equal(decl_type, assign_type)) {
                fatal("at line %d, col %d: variable assignment does not match variable type\n"
                    "Variable of type `%s` != Assignment of type `%s`",
                    line, col, decl_type.name, assign_type.name);
            }

            var_sym->type = node->var_decl.init->type;
            var_sym->is_initialized = true;
        } else {
            LOG_WARN("uninitialized variable `%s` on line %d, col %d",
                    node->var_decl.name, line, col);
        }
    }
    expect(TOKEN_SEMICOLON);

    // Set the pointer to the ASTNode for the symbol
    var_sym->node = node;

    return node;
}

static ASTNode *parse_variable_assignment(char *var_name)
{
    int line = tok()->line;
    int col = tok()->col;

    consume(); // consume `=`

    if (!symbol_table_lookup(current_scope, var_name)) {
        fatal("at line %d, col %d: unknown Symbol `%s`",
                line, col, var_name);
    }

    ASTNode *node = make_node(NODE_ASSIGN_STMT);
    node->line = line; node->col = col;
    node->assign.name = var_name;
    node->assign.value = parse_expression();

    // TODO: add typechecking to see if expression matches declared type for var

    expect(TOKEN_SEMICOLON);

    return node;
}

static ASTNode *parse_function_call(char *func_name)
{
    expect(TOKEN_SEMICOLON);
    return NULL;
}

static ASTNode *parse_block(bool in_func_toplevel)
{
    expect(TOKEN_LBRACE);

    ASTNode body = {0};
    ASTNode *cur = &body;

    ASTNode *stmt = NULL;
    while (tok()->kind != TOKEN_RBRACE) {
        switch (tok()->kind) {
            case TOKEN_RBRACE:
                break;
            case TOKEN_CONST:
                stmt = parse_variable_declaration(NULL);
                break;
            case TOKEN_IDENTIFIER:
                char *identifier = consume()->str.data;
                switch (tok()->kind) {
                    case TOKEN_LPAREN:
                        stmt = parse_function_call(identifier);
                        break;
                    case TOKEN_WALRUS:
                    case TOKEN_COLON:
                        stmt = parse_variable_declaration(identifier);
                        break;
                    case TOKEN_EQUAL:
                        stmt = parse_variable_assignment(identifier);
                        break;
                    default:
                        fatal("at line %d, col %d: invalid Token `%s` while parsing function body",
                                tok()->line, tok()->col, token_as_str(tok()->kind));
                }
                break;
            case TOKEN_IF:
            case TOKEN_ELIF:
            case TOKEN_ELSE:
                stmt = parse_conditional();
                break;
            case TOKEN_RETURN:
                consume();
                stmt = make_node(NODE_RET_STMT);
                stmt->ret_stmt.value = parse_expression();
                expect(TOKEN_SEMICOLON);
                break;
            default:
                fatal("at line %d, col %d: invalid Token `%s` while parsing function body",
                        tok()->line, tok()->col, token_as_str(tok()->kind));
        }

        if (!stmt) break;
        cur = cur->next = stmt;
    }
    expect(TOKEN_RBRACE);

    if (in_func_toplevel && (!stmt || stmt->kind != NODE_RET_STMT)) {
        ASTNode *implicit = make_node(NODE_RET_STMT);
        implicit->ret_stmt.value = NULL;
        cur = cur->next = implicit;
    }

    return body.next;
}

static ASTNode *parse_function_declaration()
{
    int line = tok()->line;
    int col = tok()->col;

    consume(); // consume keyword `func`
    char *func_name = expect(TOKEN_IDENTIFIER)->str.data;

    // Parse identifier
    ASTNode *node = make_node(NODE_FUNC_DECL);
    node->line = line; node->col = col;
    node->func_decl.name = func_name;
    node->func_decl.return_type = primitive_types[TYPE_VOID];

    // Insert function into current scope
    Symbol *func_sym = symbol_table_insert(current_scope, func_name, SYMBOL_FUNCTION);
    if (!func_sym)
        fatal("at line %d, col %d: function `%s` redeclared in scope", 
                line, col, func_name);

    // Create function scope
    SymbolTable *func_scope = symbol_table_create(func_name);

    // Insert function into its own scope for recursion
    symbol_table_insert(func_scope, func_name, SYMBOL_FUNCTION);

    // Add function scope as child of current scope
    symbol_table_add_child(current_scope, func_scope);

    // Parse parameters
    ASTNode params = {0};
    ASTNode *cur = &params;

    expect(TOKEN_LPAREN);
    while (tok()->kind != TOKEN_RPAREN) {
        char *param_name = expect(TOKEN_IDENTIFIER)->str.data;

        // Parse identifier
        ASTNode *node = make_node(NODE_VAR_DECL);
        node->var_decl.name = param_name;

        // Add paramter to function scope as a variable
        Symbol *param_sym = symbol_table_insert(func_scope, param_name, SYMBOL_VARIABLE);
        if (!param_sym) {
            fatal("at line %d, col %d: function parameter `%s` redeclared",
                    tok()->line, tok()->col, param_name);
        }

        // Parse type
        expect(TOKEN_COLON);
        node->var_decl.type = parse_type();

        // Add paramter to list
        cur = cur->next = node;

        // If there is no comma after this parameter, we are done with parsing parameters
        if (tok()->kind != TOKEN_COMMA) {
            break;
        }

        // Otherwise, consume the comma Token and keep going
        consume();
    }
    expect(TOKEN_RPAREN);
    node->func_decl.params = params.next;

    // Parse function return type (if no arrow, it's TYPE_VOID)
    if (match(TOKEN_ARROW))
        node->func_decl.return_type = parse_type();

    // Enter the function's scope
    enter_scope(func_scope);

    // Parse function body
    node->func_decl.body = parse_block(true);

    // Exit the function's scope
    exit_scope();

    func_sym->node = node;

    return node;
}

ASTNode *parse(Vector tokens)
{
    current_scope = global_scope;
    stream = tokens;

    ASTNode ast = {0};
    ASTNode *cur = &ast;

    while (tok()->kind != TOKEN_EOF) {
        ASTNode *decl = NULL;
        switch (tok()->kind) {
            case TOKEN_FUNC:
                decl = parse_function_declaration();
                break;
            case TOKEN_CONST:
                decl = parse_variable_declaration(NULL);
                break;
            case TOKEN_IDENTIFIER:
                decl = parse_variable_declaration(consume()->str.data);
                break;
            default:
                fatal("at line %d, col %d: invalid Token `%s` while parsing top-level",
                        tok()->line, tok()->col, token_as_str(tok()->kind));
        }
        cur = cur->next = decl;
    }

    // Do some checks here
    Symbol *entry_point = symbol_table_lookup(global_scope, "main");
    if (!entry_point || entry_point->kind != SYMBOL_FUNCTION) {
        LOG_ERROR("no `main` function was found!");
        fatal("failed to compile.");
    }

    return ast.next;
}

void dump_ast(ASTNode *root, int level)
{
    if (!root) return;

    static const char unary_ops[] = {
        [UN_NEG] = '-',
        [UN_NOT] = '!',
        [UN_DEREF] = '*',
        [UN_ADDR] = '&',
    };

    static const char *binary_ops[] = {
        [BIN_ADD] = "+",
        [BIN_SUB] = "-",
        [BIN_MUL] = "*",
        [BIN_DIV] = "/",
        [BIN_CMP] = "==",
        [BIN_CMP_NOT] = "!=",
        [BIN_CMP_LT] = "<",
        [BIN_CMP_GT] = ">",
        [BIN_CMP_LT_EQ] = "<=",
        [BIN_CMP_GT_EQ] = ">=",
    };

    // indent level
    printf("%*s", level, "");

    switch (root->kind) {
        case NODE_UNKNOWN:
            printf("[UNKNOWN]:\n");
            break;
        case NODE_FUNC_DECL:
            printf("[FUNC_DECL]: name = %s, return_type = %s, params = [",
                    root->func_decl.name,
                    root->func_decl.return_type.name);
            ASTNode *param = root->func_decl.params;
            for (;;) {
                if (!param) break;
                printf("%s:%s",
                        param->var_decl.name,
                        param->var_decl.type.name);
                param = param->next;
                printf("%s", param ? ", " : "");
            }
            printf("]\n");
            dump_ast(root->func_decl.body, level + 1);
            break;
        case NODE_VAR_DECL:
            printf("[VAR_DECL]: name = %s, type = %s\n",
                    root->var_decl.name,
                    root->var_decl.type.name);
            dump_ast(root->var_decl.init, level + 1);
            break;
        case NODE_RET_STMT:
            printf("[RET_STMT]:\n");
            dump_ast(root->ret_stmt.value, level + 1);
            break;
        case NODE_COND_STMT:
            printf("[COND_STMT]:\n");
            dump_ast(root->cond_stmt.expr, level + 1);
            dump_ast(root->cond_stmt.body, level + 2);
            break;
        case NODE_FUNC_CALL_EXPR:
            printf("[FUNC_CALL]:");
            break;
        case NODE_ASSIGN_STMT:
            printf("[ASSIGN]: name = %s\n", root->assign.name);
            dump_ast(root->assign.value, level + 1);
            break;
        case NODE_UNARY_EXPR:
            printf("[UNARY]: op = %c\n", 
                    unary_ops[root->unary.un_op]);
            dump_ast(root->unary.expr, level + 1);
            break;
        case NODE_BINARY_EXPR:
            printf("[BINARY]: op = %s\n", 
                    binary_ops[root->binary.bin_op]);
            dump_ast(root->binary.lhs, level + 1);
            dump_ast(root->binary.rhs, level + 1);
            break;
        case NODE_LITERAL_EXPR:
            printf("[LITERAL]: value = ");
            dump_literal(root->literal);
            printf("\n");
            break;
        case NODE_REF_EXPR:
            printf("[REF]: name = %s\n", root->ref);
            break;
        default: fatal("invalid AST! (%d)", root->kind);
    }

    dump_ast(root->next, level);
}

void dump_literal(Literal literal)
{
    switch (literal.kind) {
        case L_INT:
            printf("%ld", literal.i_val);
            break;
        case L_UINT:
        case L_SIZE:
            printf("%ld", literal.u_val);
            break;
        case L_FLOAT:
            printf("%f", literal.f_val);
            break;
        case L_DOUBLE:
            printf("%g", literal.d_val);
            break;
        case L_CHAR:
            printf("%c", literal.c_val);
            break;
        case L_BOOL:
            printf("%s", literal.b_val ? "true" : "false");
            break;
        case L_STRING:
            printf("%*s", (int)literal.s_len, literal.s_val);
            break;
        default: fatal("invalid Literal! (%d)", literal.kind);
    }
}

char *copy_literal_bytes(Literal *literal, size_t *size)
{
    char *bytes = NULL;
    size_t bytes_size = 0;

    switch (literal->kind) {
        case L_INT:
            bytes_size += sizeof(intmax_t);
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, &literal->i_val, bytes_size);
            break;
        case L_UINT:
            bytes_size += sizeof(uintmax_t);
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, &literal->u_val, bytes_size);
            break;
        case L_FLOAT:
            bytes_size += sizeof(float);
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, &literal->f_val, bytes_size);
            break;
        case L_DOUBLE:
            bytes_size += sizeof(double);
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, &literal->d_val, bytes_size);
            break;
        case L_CHAR:
            bytes_size += sizeof(char);
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, &literal->c_val, bytes_size);
            break;
        case L_BOOL:
            bytes_size += sizeof(bool);
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, &literal->b_val, bytes_size);
            break;
        case L_STRING:
            bytes_size += literal->s_len;
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, literal->s_val, bytes_size);
            break;
        case L_SIZE:
            bytes_size += sizeof(size_t);
            bytes = calloc(bytes_size, sizeof(char));
            memcpy(bytes, &literal->size, bytes_size);
            break;
    }

    *size = bytes_size;
    return bytes;
}

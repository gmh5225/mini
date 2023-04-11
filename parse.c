#include "mini.h"

static symbol_table *current_scope;  // The current scope of the parser
static token_stream *tokens;         // The stream of tokens to parse
static ast_node *stack_top;             // The top of the expression stack

void enter_scope(symbol_table *new_scope) {
    current_scope = new_scope;
}

void exit_scope() {
    current_scope = current_scope->parent;
}

static token *tok() {
    return token_stream_get(tokens);
}

static token *consume() {
    return token_stream_next(tokens);
}

static token *match(token_kind want) {
    bool matches = tok()->kind == want;
    if (matches) return consume();
    return NULL;
}

static token *expect(token_kind expected) {
    token *curr = consume();
    if (curr->kind != expected) {
        error_at_token(curr, "expected `%s`, got `%s`",
                token_as_str(expected), token_as_str(curr->kind));
    }
    return curr;
}

static ast_node *make_node(node_kind kind) {
    ast_node *node = calloc(1, sizeof(struct ast_node));
    node->kind = kind;
    return node;
}

void push_expr_node(ast_node *expr) {
    if (!stack_top) {
        stack_top = expr;
    } else {
        ast_node *temp = stack_top;
        stack_top = expr;
        stack_top->next = temp;
    }
}

ast_node *pop_expr_node() {
    ast_node *ret = stack_top;
    stack_top = stack_top->next;
    // Propagate inner-most expression type to outer expressions
    if (stack_top) {
        stack_top->type = ret->type;
    }
    return ret;
}

static void parse_factor();
static void parse_term();

static ast_node *parse_unary_expr(char un_op) {
    ast_node *node = make_node(NODE_UNARY_EXPR);
    node->unary.un_op = un_op;
    node->unary.expr = pop_expr_node();
    return node;
}

static ast_node *parse_binary_expr(char bin_op) {
    ast_node *node = make_node(NODE_BINARY_EXPR);
    node->binary.bin_op = bin_op;

    node->binary.lhs = pop_expr_node();
    parse_term(tokens);
    node->binary.rhs = pop_expr_node();

    // TODO: add typechecking here
    //type lhs_type = node->binary.lhs->type;
    //type rhs_type = node->binary.rhs->type;
    //if (!types_equal(lhs_type, rhs_type)) {
    //    error_at_token(tokens, "type mismatch in binary expression");
    //}

    return node;
}

static void parse_factor() {
    ast_node *node = make_node(NODE_LITERAL_EXPR);

    switch (tok()->kind) {
        case TOKEN_IDENTIFIER:
            // Check to see if the variable we are referencing is valid
            char *var_name = consume()->str.data;
            symbol *var_sym = symbol_table_lookup(current_scope, var_name);
            if (!var_sym) {
                error_at_token(tok(), "unknown symbol `%s`", var_name);
            }
            node->kind = NODE_REF_EXPR;
            node->type = var_sym->type;
            node->ref = var_name;
            break;
        case TOKEN_NUMBER:
            // TODO: Infer type from number here.
            // For now, we just assume its an `int`
            node->type = primitive_types[TYPE_INT];
            node->literal.i_val = consume()->i_val;
            break;
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            node->type = primitive_types[TYPE_BOOL];
            node->literal.b_val = consume()->b_val;
            break;
        default:
            error_at_token(tok(), "invalid token `%s` while parsing expression",
                    token_as_str(tok()->kind));
    }

    push_expr_node(node);
}

void parse_term() {
    parse_factor();
    for (;;) {
        char bin_op = 0;
        switch (tok()->kind) {
            case TOKEN_STAR:
                bin_op = '*'; consume();
                break;
            case TOKEN_SLASH:
                bin_op = '/'; consume();
                break;
            default: break;
        }

        if (bin_op == 0) break;
        push_expr_node(parse_binary_expr(bin_op));
    }
}

static ast_node *parse_expression() {
    char un_op = 0;
    switch (tok()->kind) {
        case TOKEN_MINUS: 
            un_op = '-'; consume();
            break;
        case TOKEN_BANG: 
            un_op = '!'; consume();
            break;
        case TOKEN_STAR: 
            un_op = '*'; consume();
            break;
        default: break;
    }

    parse_term();

    if (un_op > 0) {
        push_expr_node(parse_unary_expr(un_op));
    }

    for (;;) {
        char bin_op = 0;
        switch (tok()->kind) {
            case TOKEN_PLUS:
                bin_op = '+'; consume();
                break;
            case TOKEN_MINUS:
                bin_op = '-'; consume();
                break;
            default: break;
        }

        if (bin_op == 0) break;
        push_expr_node(parse_binary_expr(bin_op));
    }

    return pop_expr_node();
}

static type parse_type() {
    char *type_name = expect(TOKEN_IDENTIFIER)->str.data;

    // Search for type symbol in current scope
    symbol *type_sym = symbol_table_lookup(current_scope, type_name);
    if (!type_sym) {
        error_at_token(tok(), "unknown type `%s`", type_name);
    }
    return type_sym->type;
}

static ast_node *parse_variable_declaration(char *var_name) {
    // Check if the variable is a constant and parse identifier if not yet parsed
    bool is_constant = false;
    if (!var_name) {
        expect(TOKEN_CONST);
        is_constant = true;
        var_name = expect(TOKEN_IDENTIFIER)->str.data;
    }

    ast_node *node = make_node(NODE_VAR_DECL);
    node->var_decl.name = var_name;
    node->var_decl.type = primitive_types[TYPE_VOID];

    // Insert variable into current scope
    symbol *var_sym = symbol_table_insert(current_scope, var_name, SYMBOL_VARIABLE);
    if (!var_sym) {
        error_at_token(tok(), "variable `%s` redeclared in scope", var_name);
    }
    var_sym->is_constant = is_constant;

    // Parse assignment and/or type declaration of variable
    if (match(TOKEN_WALRUS)) {
        // num := -1 + 2 * 3
        node->var_decl.init = parse_expression();
        var_sym->type = node->var_decl.init->type;
        var_sym->is_initialized = true;
    } else {
        expect(TOKEN_COLON);
        node->var_decl.type = parse_type();

        if (match(TOKEN_EQUAL)) {
            node->var_decl.init = parse_expression();
            var_sym->type = node->var_decl.init->type;
            var_sym->is_initialized = true;
        }
    }

    expect(TOKEN_SEMICOLON);
    return node;
}

static ast_node *parse_variable_assignment(char *var_name) {
    consume(); // consume `=`

    ast_node *node = make_node(NODE_ASSIGN_EXPR);
    node->assign.name = var_name;
    node->assign.value = parse_expression();
    
    // TODO: add typechecking to see if expression matches declared type for var

    expect(TOKEN_SEMICOLON);
    return node;
}

static ast_node *parse_function_call(char *func_name) {
    return NULL;
}

static ast_node *parse_function_declaration() {
    consume(); // consume keyword `func`
    char *func_name = expect(TOKEN_IDENTIFIER)->str.data;

    // Parse identifier
    ast_node *node = make_node(NODE_FUNC_DECL);
    node->func_decl.name = func_name;
    node->func_decl.return_type = primitive_types[TYPE_VOID];

    // Insert function into current scope
    symbol *func_sym = symbol_table_insert(current_scope, func_name, SYMBOL_FUNCTION);
    if (!func_sym) {
        error_at_token(tok(), "function `%s` redeclared in scope", func_name);
    }

    // Create function scope
    symbol_table *func_scope = symbol_table_create(func_name);

    // Insert function into its own scope for recursion
    symbol_table_insert(func_scope, func_name, SYMBOL_FUNCTION);

    // Add function scope as child of current scope
    symbol_table_add_child(current_scope, func_scope);

    // Parse parameters
    ast_node params = {0};
    ast_node *cur = &params;

    expect(TOKEN_LPAREN);
    while (tok()->kind != TOKEN_RPAREN) {
        char *param_name = expect(TOKEN_IDENTIFIER)->str.data;

        // Parse identifier
        ast_node *node = make_node(NODE_VAR_DECL);
        node->var_decl.name = param_name;

        // Add paramter to function scope as a variable
        symbol *param_sym = symbol_table_insert(func_scope, param_name, SYMBOL_VARIABLE);
        if (!param_sym) {
            error_at_token(tok(), "function parameter `%s` redeclared");
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

        // Otherwise, consume the comma token and keep going
        consume();
    }
    expect(TOKEN_RPAREN);
    node->func_decl.params = params.next;

    // Parse function return type (if no arrow, it's TYPE_VOID)
    if (match(TOKEN_ARROW)) {
        node->func_decl.return_type = parse_type();
    }

    // Parse function body
    expect(TOKEN_LBRACE);

    // Enter the function's scope
    enter_scope(func_scope);

    ast_node body = {0};
    cur = &body;

    while (tok()->kind != TOKEN_RBRACE) {
        ast_node *stmt = NULL;
        switch (tok()->kind) {
            case TOKEN_RBRACE: break;
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
                        stmt = parse_variable_declaration(identifier);
                        break;
                    case TOKEN_EQUAL:
                        stmt = parse_variable_assignment(identifier);
                        break;
                    default:
                        error_at_token(tok(), "invalid token `%s` while parsing function body",
                                token_as_str(tok()->kind));
                }
                break;
            case TOKEN_RETURN:
                consume();
                stmt = make_node(NODE_RET_STMT);
                stmt->ret_stmt.value = parse_expression();
                expect(TOKEN_SEMICOLON);
                break;
            default:
                error_at_token(tok(), "invalid token `%s` while parsing function body",
                        token_as_str(tok()->kind));
        }

        if (!stmt) break;
        cur = cur->next = stmt;
    }
    expect(TOKEN_RBRACE);
    node->func_decl.body = body.next;

    // Exit the function's scope
    exit_scope();

    return node;
}

ast_node *parse(token_stream *stream) {
    current_scope = global_scope;
    tokens = stream;

    ast_node ast = {0};
    ast_node *cur = &ast;

    while (tok()->kind != TOKEN_EOF) {
        ast_node *decl = NULL;
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
                error_at_token(tok(), "invalid token `%s` while parsing top-level",
                        token_as_str(tok()->kind));
        }
        cur = cur->next = decl;
    }

    return ast.next;
}

static void dump_ast_impl(ast_node *root, int level) {
    if (!root) return;

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
            ast_node *param = root->func_decl.params;
            for (;;) {
                if (!param) break;
                printf("%s:%s",
                        param->var_decl.name,
                        param->var_decl.type.name);
                param = param->next;
                printf("%s", param ? ", " : "");
            }
            printf("]\n");
            dump_ast_impl(root->func_decl.body, level + 1);
            break;
        case NODE_VAR_DECL:
            printf("[VAR_DECL]: name = %s, type = %s\n",
                    root->var_decl.name,
                    root->var_decl.type.name);
            dump_ast_impl(root->var_decl.init, level + 1);
            break;
        case NODE_RET_STMT:
            printf("[RET_STMT]:\n");
            dump_ast_impl(root->ret_stmt.value, level + 1);
            break;
        case NODE_FUNC_CALL_EXPR:
            printf("[FUNC_CALL]:");
            break;
        case NODE_ASSIGN_EXPR:
            printf("[ASSIGN]: name = %s\n", root->assign.name);
            dump_ast_impl(root->assign.value, level + 1);
            break;
        case NODE_UNARY_EXPR:
            printf("[UNARY]: op = %c\n", root->unary.un_op);
            dump_ast_impl(root->unary.expr, level + 1);
            break;
        case NODE_BINARY_EXPR:
            printf("[BINARY]: op = %c\n", root->binary.bin_op);
            dump_ast_impl(root->binary.lhs, level + 1);
            dump_ast_impl(root->binary.rhs, level + 1);
            break;
        case NODE_LITERAL_EXPR:
            printf("[LITERAL]: value = ");
            switch (root->type.kind) {
                case TYPE_INT:
                    printf("INT(%ld)", root->literal.i_val);
                    break;
                case TYPE_BOOL:
                    printf("BOOL(%s)", root->literal.b_val ? "true" : "false");
                    break;
                default: error("invalid literal type!");
            }
            printf("\n");
            break;
        case NODE_REF_EXPR:
            printf("[REF]: name = %s\n", root->ref);
            break;
        default: error("invalid AST!");
    }

    dump_ast_impl(root->next, level);
}

void dump_ast(ast_node *program) {
    dump_ast_impl(program, 0);
    printf("\n");
}

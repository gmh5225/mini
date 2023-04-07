#include "lexer.h"
#include "util.h"

#include <stdbool.h>
#include <string.h>

#include <stdio.h>

const char *token_strings[] = {
    [TOKEN_UNKNOWN] = "[UNKNOWN]",
    [TOKEN_EOF] = "[EOF]",
    [TOKEN_CONST] = "const",
    [TOKEN_FUNC] = "func",
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
    [TOKEN_WALRUS] = ":=",
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

const char *token_as_str(token_type type) {
    return token_strings[type];
}

static char *p = NULL;
static char *start = NULL;
int line_no = 1;
int col_no = 1;

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

bool next_char_matches(char c) {
    char next = *(p + 1);
    if (next == '\0') {
        fail("expected another character, got EOF");
    }
    bool matches = next == c;
    if (matches) p++;
    return matches;
}

void lex_alphabetic(token *t) {
    while (is_alphanumeric(*p)) { p++; col_no++; }
    size_t length = (size_t)(p - start);

    for (token_type type = TOKEN_CONST; type <= TOKEN_FALSE; type++) {
        if (memcmp(start, token_as_str(type), length) == 0) {
            start = p;
            t->type = type;
            return;
        }
    }

    t->type = TOKEN_IDENTIFIER;
    t->lexeme = start; t->length = length;
}

void lex_numeric(token *t) {
    while (is_numeric(*p)) { p++; col_no++; }
    size_t length = (size_t)(p - start);

    t->type = TOKEN_NUMBER;
    t->lexeme = start; t->length = length;
}

void lex_symbol(token *t) {
    switch (*p) {
        case '+': t->type = TOKEN_PLUS; break;
        case '-': t->type = TOKEN_MINUS; break;
        case '*': t->type = TOKEN_STAR; break;
        case '/': t->type = TOKEN_SLASH; break;
        case '=': t->type = TOKEN_EQUAL; break;
        case '!': t->type = TOKEN_BANG; break;
        case ';': t->type = TOKEN_SEMICOLON; break;
        case ':': t->type = next_char_matches('=') ? TOKEN_WALRUS : TOKEN_COLON; break;
        case ',': t->type = TOKEN_COMMA; break;
        case '.': t->type = TOKEN_DOT; break;
        case '{': t->type = TOKEN_LBRACE; break;
        case '}': t->type = TOKEN_RBRACE; break;
        case '(': t->type = TOKEN_LPAREN; break;
        case ')': t->type = TOKEN_RPAREN; break;
        case '[': t->type = TOKEN_LBRACKET; break;
        case ']': t->type = TOKEN_RBRACKET; break;
        default: fail("unknown symbol at line %zu, col %zu: %c", line_no, col_no, *p);
    }
    p++; col_no++;
}

token next_token(char *input) {
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
    if (*p == '/') {
        p++;
        if (*p == '/') {
            while (*p != '\n') { p++; } p++;
            line_no++; col_no = 1;
            start = p;
            return next_token(input);
        } else if (*p == '*') {
            p++;
            for (;;) {
                if (*p == '*' && next_char_matches('/')) {
                    p++;
                    break;
                }
                p++;
                if (*p == '\n') {
                    line_no++; col_no = 1;
                } else {
                    col_no++;
                }
            }
            start = p;
            return next_token(input);
        } else {
            fail("invalid token at line %zu, col %zu: %c", line_no, col_no, *p);
        }
    }

    start = p;
    token result = make_token(TOKEN_UNKNOWN);
    if (is_alphabetic(*p)) {
        lex_alphabetic(&result);
    } else if (is_numeric(*p)) {
        lex_numeric(&result);
    } else {
        lex_symbol(&result);
    }
    start = p;
    return result;
}

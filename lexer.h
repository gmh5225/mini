#ifndef MINI_LEXER_H
#define MINI_LEXER_H

#include <stddef.h>

typedef enum {
    /* Misc */
    TOKEN_UNKNOWN,
    TOKEN_EOF,
    /* Keywords */
    TOKEN_CONST,
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
    /* Primitive Types */
    TOKEN_INT,
    TOKEN_BOOL,
    /* Operators */
    TOKEN_WALRUS,
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

token next_token();
const char *token_as_str(token_type type);

extern const char *token_strings[];

#endif

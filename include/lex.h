#ifndef MINI_LEX_H
#define MINI_LEX_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

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

#endif

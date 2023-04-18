#ifndef MINI_LEX_H
#define MINI_LEX_H

#include "vector.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

typedef enum TokenKind TokenKind;
enum TokenKind
{
    TOKEN_UNKNOWN = 0, // Misc
    TOKEN_EOF,
    TOKEN_CONST, // Keywords
    TOKEN_RETURN,
    TOKEN_FUNC,
    TOKEN_IMPORT,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_INT,    // Primitive types
    TOKEN_BOOL,
    TOKEN_WALRUS, // Operators
    TOKEN_EQUAL,
    TOKEN_BANG,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_DOUBLE_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS_THAN_EQUAL,
    TOKEN_GREATER_THAN_EQUAL,
    TOKEN_LANGLE,
    TOKEN_RANGLE,
    TOKEN_LBRACE, // symbols
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
    TOKEN_ARROW,
    TOKEN_IDENTIFIER, // User-defined
    TOKEN_NUMBER,
};

extern const char *token_strings[];
const char *token_as_str(TokenKind kind);

#define IDENTIFIER_MAX_LEN  256
#define NUMBER_MAX_LEN      256
#define STRING_MAX_LEN      4096

typedef struct Token Token;
struct Token
{
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

Vector lex(FILE *file); // Token

#endif

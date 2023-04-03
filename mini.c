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

/* Type definitions */
typedef enum {
    /* Misc */
    TOKEN_UNKNOWN,
    TOKEN_EOF,
    /* Keywords */
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_GOTO,
    TOKEN_TRUE,
    TOKEN_FALSE,
    /* Primitive Types */
    TOKEN_INT,
    TOKEN_BOOL,
    /* Operators */
    TOKEN_EQUAL,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    /* Symbols */
    TOKEN_SEMICOLON,
    TOKEN_COLON,
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
} token;

const char *token_strings[] = {
    [TOKEN_UNKNOWN] = "< UNKNOWN >",
    [TOKEN_EOF] = "< EOF >",
    [TOKEN_LET] = "let",
    [TOKEN_CONST] = "const",
    [TOKEN_GOTO] = "goto",
    [TOKEN_TRUE] = "true",
    [TOKEN_FALSE] = "false",
    [TOKEN_INT] = "int",
    [TOKEN_BOOL] = "bool",
    [TOKEN_EQUAL] = "=",
    [TOKEN_PLUS] = "+",
    [TOKEN_MINUS] = "-",
    [TOKEN_STAR] = "*",
    [TOKEN_SLASH] = "/",
    [TOKEN_SEMICOLON] = ";",
    [TOKEN_COLON] = ":",
    [TOKEN_DOT] = ".",
    [TOKEN_COMMA] = ",",
    [TOKEN_IDENTIFIER] = "< IDENTIFIER >",
    [TOKEN_NUMBER] = "< NUMBER >",
};

typedef enum {
    NODE_VARIABLE,
    NODE_FUNCTION,
    NODE_FUNCTION_CALL,
    NODE_CONDITIONAL,
    NODE_EXPRESSION,
    NODE_LITERAL,
} ast_node_type;

typedef struct ast_node {
    ast_node_type type;
    char *value;
    struct ast_node **children;
    size_t num_children;
} ast_node;

token current_token = {0};
token previous_token = {0};
ast_node *ast_root = NULL;

token next_token();
bool match(token_type expected);
void fail_with_context(const char *fmt, ...);

ast_node *parse_variable(token **tokens);
ast_node *parse_function(token **tokens);
ast_node *parse_function_call(token **tokens);
ast_node *parse_conditional(token **tokens);
ast_node *parse_expression(token **tokens);
ast_node *parse_literal(token **tokens);

int main(int argc, char **argv) {
    char *input_filename = NULL;
    char *output_filename = "a.out";

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
        } else {
            input_filename = arg;
        }
    }
    if (!input_filename) {
        fail("input file is required");
    }

    char *source_code = read_whole_file(input_filename);
    printf("outputting to: %s\n", output_filename);

    token token = {0};
    do {
        token = next_token(source_code);
        const char *token_as_str = token_strings[token.type];
        printf("%s", token_as_str);
        if (token.length > 0) {
            printf(" = %.*s", token.length, token.lexeme);
        }
        printf("\n");
    } while (token.type != TOKEN_EOF);

    if (source_code) {
        free(source_code);
    }
    return 0;
}

static bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static bool is_alphabetic(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static bool is_numeric(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alphanumeric(char c) {
    return is_alphabetic(c) || is_numeric(c);
}

int str_to_int(const char *s, size_t length) {
    int n = 0;
    for (size_t i = 0; i < length; i++) {
        n = n * 10 + (s[i] - '0');
    }
    return n;
}

token next_token(char *input) {
    static char *p = NULL;
    static char *start = NULL;
    static int line_no = 1, col_no = 1;

    if (!p) {
        p = input;
        start = input;
    }

    // Skip whitespace
    while (is_whitespace(*p)) {
        if (*p == '\n') { line_no++; col_no = 1; }
        p++; col_no++;
    }

    // Check for end of source
    if (*p == '\0') { return (token){.type = TOKEN_EOF}; }

    // Skip comments
    if (*p == '#') {
        while (*p != '\n') { p++; }
        line_no++;
        col_no = 1;
        start = p;
        return next_token(input);
    }

    if (is_alphabetic(*p)) {
        while (is_alphanumeric(*p)) { p++; col_no++; }
        size_t length = (size_t)(p - start);

        for (token_type type = TOKEN_LET; type <= TOKEN_FALSE; type++) {
            if (memcmp(start, token_strings[type], length) == 0) {
                token keyword = (token){.type = type};
                start = p;
                return keyword;
            }
        }

        token identifier = (token){.type = TOKEN_IDENTIFIER, .lexeme = start, .length = length};
        start = p;
        return identifier;
    } else if (is_numeric(*p)) {
        while (is_numeric(*p)) { p++; col_no++; }
        size_t length = (size_t)(p - start);

        token number = (token){.type = TOKEN_NUMBER, .lexeme = start, .length = length};
        start = p;
        return number;
    }

    token symbol;
    switch (*p) {
        case '+': symbol = (token){.type = TOKEN_PLUS}; break;
        case '-': symbol = (token){.type = TOKEN_MINUS}; break;
        case '*': symbol = (token){.type = TOKEN_STAR}; break;
        case '/': symbol = (token){.type = TOKEN_SLASH}; break;
        case '=': symbol = (token){.type = TOKEN_EQUAL}; break;
        case ';': symbol = (token){.type = TOKEN_SEMICOLON}; break;
        default: fail("unknown symbol at line %zu, col %zu: %c", line_no, col_no, *p);
    }

    *p++; col_no++;
    start = p;
    return symbol;
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

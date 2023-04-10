#include "mini.h"

#define LEX_BUF_SZ              4096
#define TOKEN_STREAM_CAPACITY   1024

enum { END_OF_BUF = 0, END_OF_FILE };

const char *token_strings[] = {
    [TOKEN_UNKNOWN] = "[UNKNOWN]",
    [TOKEN_EOF] = "[EOF]",
    [TOKEN_CONST] = "const",
    [TOKEN_FUNC] = "func",
    [TOKEN_RETURN] = "return",
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
    [TOKEN_DOT] = ".",
    [TOKEN_COMMA] = ",",
    [TOKEN_ARROW] = "->",
    [TOKEN_IDENTIFIER] = "[IDENTIFIER]",
    [TOKEN_NUMBER] = "[NUMBER]",
};
const char *token_as_str(TokenKind kind) { return token_strings[kind]; }

TokenStream token_stream_create() {
    TokenStream stream = {0};
    stream.pos = 0;
    stream.size = 0;
    stream.capacity = TOKEN_STREAM_CAPACITY;
    stream.tokens = calloc(TOKEN_STREAM_CAPACITY, sizeof(Token));
    return stream;
}

void token_stream_append(TokenStream *stream, Token token) {
    if (stream->size == stream->capacity) {
        stream->capacity <<= 1;
        void *tmp = realloc(stream->tokens, sizeof(Token) * stream->capacity);
        stream->tokens = tmp;
    }
    stream->tokens[stream->size++] = token;
}

Token *token_stream_get(TokenStream *stream) {
    if (stream->pos >= stream->size) return NULL;
    return &stream->tokens[stream->pos];
}

Token *token_stream_next(TokenStream *stream) {
    if (stream->pos >= stream->size) return NULL;
    return &stream->tokens[stream->pos++];
}

Token *token_stream_prev(TokenStream *stream) {
    return NULL;
}

// TODO: Add support for UTF-8 codepoints

// Lexer State
static FILE *current_file;      // file pointer of source file to tokenize
static char buf[LEX_BUF_SZ];    // source file buffer
static size_t pos;              // position of lexer within `buf`
static size_t nread;            // # bytes last read from `current_file`
static int line;                // current line no
static int col;                 // current col no
static int line_start;          // pos at where the current line starts in `buf`

static void fill_buffer() {
    memset(buf, 0, sizeof(char) * LEX_BUF_SZ);
    nread = fread(buf, sizeof(char), LEX_BUF_SZ, current_file);
    pos = 0;
}

static bool reached(int ctrl) {
    if (pos != nread - 1) return false;
    return ctrl ? (nread < LEX_BUF_SZ) : (nread == LEX_BUF_SZ);
}

static char next() {
    if (reached(END_OF_BUF)) {
        fill_buffer();
    } else if (reached(END_OF_FILE)) {
        error("lexer was expecting more characters but reached EOF! (next)");
    }

    char c = buf[pos];
    if (c == '\n') {
        line_start = pos + 1;
        line++; col = 1;
    } else {
        col++;
    }
    pos++;

    return c;
}

static char peek() {
    if (reached(END_OF_BUF)) {
        fill_buffer();
    } else if (reached(END_OF_FILE)) {
        error("lexer was expected more characters but reached EOF! (peek)");
    }
    return buf[pos];
}

static bool match(char expected) {
    bool matches = peek() == expected;
    if (matches) { next(); }
    return matches;
}

static Token make_token(TokenKind kind) {
    Token token = {0};
    token.kind = kind;
    token.line = line;
    token.col = col;
    return token;
}

static bool is_whitespace(char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }
static bool is_alphabetic(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'; }
static bool is_numeric(char c) { return c >= '0' && c <= '9'; }
static bool is_alphanumeric(char c) { return is_alphabetic(c) || is_numeric(c); }

static Token lex_alphabetic() {
    Token token = make_token(TOKEN_IDENTIFIER);
    char buf[IDENTIFIER_MAX_LEN] = {0};
    size_t len = 0;
    while (!reached(END_OF_FILE)) {
        char c = peek();
        if (!is_alphanumeric(c) && c != '_') break;

        if (len >= IDENTIFIER_MAX_LEN) {
            error_at(line, col, "identifier is too long (max = %d)", IDENTIFIER_MAX_LEN);
        }

        buf[len++] = c;
        next();
    }
    buf[len] = 0;

    // Check if token is a keyword
    for (TokenKind kind = TOKEN_CONST; kind <= TOKEN_FALSE; kind++) {
        if (memcmp(buf, token_as_str(kind), len) == 0) {
            token.kind = kind;
            return token;
        }
    }

    // Token must be an identifier
    token.str.data = calloc(len, sizeof(char));
    token.str.length = len - 1;
    memcpy(token.str.data, buf, len);

    return token;
}

// TODO: add support for binary/octal/hexadecimal numbers + floating point numbers
static Token lex_numeric() {
    Token token = make_token(TOKEN_NUMBER);
    char buf[NUMBER_MAX_LEN] = {0};
    size_t len = 0;
    while (!reached(END_OF_FILE)) {
        char c = peek();
        if (!is_numeric(c)) break;

        if (len >= NUMBER_MAX_LEN) {
            error_at(line, col, "number is too long (max = %d)", NUMBER_MAX_LEN);
        }

        buf[len++] = c;
        next();
    }
    buf[len] = 0;

    // Token must be an identifier
    token.kind = TOKEN_NUMBER;
    token.i_val = str_to_int(buf, len - 1);
    return token;
}

TokenStream lex(FILE *file) {
    // Initialize Lexer State
    current_file = file;
    line = col = 1;
    pos = nread = 0;
    memset(buf, 0, sizeof(char) * LEX_BUF_SZ);
    fill_buffer();

    // Tokenize
    TokenStream stream = token_stream_create();
    while (!reached(END_OF_FILE)) {
        char c = peek();

        // Skip whitespace
        if (is_whitespace(c)) {
            for (;;) {
                if (match(' ') || match('\n') || 
                    match('\r') || match('\t')) break;
                next();
            }
            continue;
        }

        // Skip comments
        if (match('/')) {
            // Single-line
            if (match('/')) {
                for (;;) {
                    if (match('\n')) break;
                    next();
                }
                continue;
            }

            // Multi-line
            if (match('*')) {
                for (;;) {
                    if (match('*') && match('/')) break;
                    next();
                }
                continue;
            }
        }

        if (is_alphabetic(c)) {
            token_stream_append(&stream, lex_alphabetic());
            continue;
        }

        if (is_numeric(c)) {
            token_stream_append(&stream, lex_numeric());
            continue;
        }

        int sym_line = line;
        int sym_col = col;
        Token sym;
        switch (next()) {
            case '+': sym = make_token(TOKEN_PLUS); break;
            case '-': sym = make_token(match('>') ? TOKEN_ARROW : TOKEN_MINUS); break;
            case '*': sym = make_token(TOKEN_STAR); break;
            case '/': sym = make_token(TOKEN_SLASH); break;
            case '=': sym = make_token(TOKEN_EQUAL); break;
            case '!': sym = make_token(TOKEN_BANG); break;
            case ';': sym = make_token(TOKEN_SEMICOLON); break;
            case ':': sym = make_token(match('=') ? TOKEN_WALRUS : TOKEN_COLON); break;
            case ',': sym = make_token(TOKEN_COMMA); break;
            case '.': sym = make_token(TOKEN_DOT); break;
            case '{': sym = make_token(TOKEN_LBRACE); break;
            case '}': sym = make_token(TOKEN_RBRACE); break;
            case '(': sym = make_token(TOKEN_LPAREN); break;
            case ')': sym = make_token(TOKEN_RPAREN); break;
            case '[': sym = make_token(TOKEN_LBRACKET); break;
            case ']': sym = make_token(TOKEN_RBRACKET); break;
            default: error_at(sym_line, sym_col, "unknown symbol `%c`", c);
        }
        sym.line = sym_line;
        sym.col = sym_col;
        token_stream_append(&stream, sym);
    }

    token_stream_append(&stream, make_token(TOKEN_EOF));
    return stream;
}

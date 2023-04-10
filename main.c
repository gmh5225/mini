/* mini compiler */
#include "mini.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    char *input_filename = NULL;
    char *output_filename = "a.out";

    // Parse command line arguments 
    // TODO: put this in a function
    bool skip = false;
    for (int i = 1; i < argc; i++) {
        if (skip) { skip = false; continue; }
        char *arg = argv[i];
        if (strcmp(arg, "-o") == 0) {
            if (i + 1 >= argc) {
                error("not enough arguments for option '%s'", arg);
            }
            output_filename = argv[i + 1];
            skip = true;
        } else {
            input_filename = arg;
        }
    }
    if (!input_filename) {
        error("input file is required");
    }

    FILE *file = fopen(input_filename, "rb");
    if (!file) {
        error("couldn't open file '%s'", file);
    }

    init_global_scope();
    TokenStream stream = lex(file);

#ifdef DEBUG
    for (;;) {
        Token *t = token_stream_next(&stream);
        if (t->kind == TOKEN_EOF) break;
        printf("%s\n", token_as_str(t->kind));
    }
    stream.pos = 0;
#endif

    Node *ast = parse(&stream);
#ifdef DEBUG
    while (ast) {
        printf("node_id: %d\n", ast->kind);
        ast = ast->next;
    }
#endif

    return 0;
}

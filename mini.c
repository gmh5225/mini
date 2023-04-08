/* mini compiler */
#include "codegen.h"
#include "lexer.h"
#include "parser.h"
#include "symbol_table.h"
#include "util.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MINI_VERSION "0.1.0"

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
    global_scope = symbol_table_create("__GLOBAL__");
    ast_node *ast = parse_program(source_code);

#ifdef DEBUG
    printf("AST for file '%s':\n", input_filename);
    ast_dump(ast);
    printf("\n");

    printf("Symbol Table(s):\n");
    symbol_table_dump(global_scope);
    printf("\n");
#endif

    target_asm output = {0};
    target_asm_init(&output, TARGET_LINUX_NASM_X86_64);

    printf("compiling to %s\n", output_filename);
    target_asm_generate_code(&output, ast);
    target_asm_write_to_file(&output, output_filename);

    target_asm_free(&output);
    free(source_code);
    return 0;
}

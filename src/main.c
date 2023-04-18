#include "lex.h"
#include "parse.h"
#include "ir.h"
#include "symbols.h"
#include "util.h"
#include "vector.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum 
{ 
    DUMP_TOKENS = 1 << 1, 
    DUMP_AST = 1 << 2,
    DUMP_SYMBOLS = 1 << 3,
    DUMP_IR = 1 << 4,
};

typedef struct MCCOpts MCCOpts;
struct MCCOpts
{
    int dump_flags;
    char *input_filename;
    char *output_filename;
};

MCCOpts parse_mcc_options(int argc, char **argv)
{
    MCCOpts opts = {
        .dump_flags = 0,
        .input_filename = NULL,
        .output_filename = "a.out",
    };

    bool skip = false;
    for (int i = 1; i < argc; i++) {
        if (skip) { skip = false; continue; }

        char *arg = argv[i];

        if (strcmp(arg, "-o") == 0) {
            if (i + 1 >= argc) {
                error("not enough arguments for option '%s'", arg);
            }
            opts.output_filename = argv[i + 1];
            skip = true;
            continue;
        }

        if (strcmp(arg, "-dT") == 0) {
            opts.dump_flags |= DUMP_TOKENS;
            continue;
        }

        if (strcmp(arg, "-dA") == 0) {
            opts.dump_flags |= DUMP_AST;
            continue;
        }

        if (strcmp(arg, "-dSY") == 0) {
            opts.dump_flags |= DUMP_SYMBOLS;
            continue;
        }

        if (strcmp(arg, "-dIR") == 0) {
            opts.dump_flags |= DUMP_IR;
            continue;
        }

        opts.input_filename = arg;
    }

    if (!opts.input_filename) {
        error("input file is required");
    }

    return opts;
}

int main(int argc, char **argv)
{
    MCCOpts opts = parse_mcc_options(argc, argv);
    srand(time(NULL));

    FILE *file = fopen(opts.input_filename, "rb");
    if (!file) {
        error("couldn't open file `%s`", file);
    }

    init_global_scope();

    Vector tokens = lex(file);
    if (opts.dump_flags & DUMP_TOKENS) {
        for (size_t i = 0; i < tokens.size; i++) {
            Token *token = (Token *)vector_get(&tokens, i);
            if (token->kind == TOKEN_EOF) break;
            printf("%s\n", token_as_str(token->kind));
        }
    }
    fclose(file);

    ASTNode *ast = parse(tokens);
    if (opts.dump_flags & DUMP_AST) {
        dump_ast(ast, 0);
    }

    if (opts.dump_flags & DUMP_SYMBOLS) {
        symbol_table_dump(global_scope, 0);
    }

    Program program = emit_ir(ast);
    if (opts.dump_flags & DUMP_IR) {
        BasicBlock *block = program.blocks;
        while (block) {
            printf("[BasicBlock %s#%d] (%ld predecessors, %ld successors, %ld instructions)\n",
                    block->tag, block->id,
                    block->predecessors.size,
                    block->successors.size,
                    block->instructions.size);

            for (size_t i = 0; i < block->instructions.size; i++) {
                Instruction *inst = (Instruction *)vector_get(&block->instructions, i);
                dump_instruction(inst);
            }

            block = block->next;
        }
    }

    printf("compiling to `%s`\n", opts.output_filename);
    return 0;
}

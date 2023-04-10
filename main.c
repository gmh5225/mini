/* mini compiler */
#include "mini.h"

enum { DUMP_TOKENS = 1, DUMP_AST };

MiniOptions parse_mini_options(int argc, char **argv) {
    MiniOptions opts = {
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

        if (strcmp(arg, "-T") == 0) {
            opts.dump_flags |= DUMP_TOKENS;
            continue;
        }

        if (strcmp(arg, "-A") == 0) {
            opts.dump_flags |= DUMP_AST;
            continue;
        }

        opts.input_filename = arg;
    }

    if (!opts.input_filename) {
        error("input file is required");
    }

    return opts;
}

int main(int argc, char **argv) {
    MiniOptions opts = parse_mini_options(argc, argv);

    FILE *file = fopen(opts.input_filename, "rb");
    if (!file) {
        error("couldn't open file `%s`", file);
    }

    init_global_scope();
    TokenStream stream = lex(file);
    fclose(file);

    if (opts.dump_flags & DUMP_TOKENS) {
        for (;;) {
            Token *t = token_stream_next(&stream);
            if (t->kind == TOKEN_EOF) break;
            printf("%s\n", token_as_str(t->kind));
        }
        stream.pos = 0; // reset stream pos after printing tokens
    }

    Node *program = parse(&stream);

    if (opts.dump_flags & DUMP_AST) {
        dump_ast(program);
    }

    printf("compiling to `%s`\n", opts.output_filename);
    TargetASM target = {0};
    target_asm_init(&target, TARGET_LINUX_NASM_X86_64);
    target_asm_generate_code(&target, program);

    target_asm_write_to_file(&target, opts.output_filename);
    target_asm_free(&target);

    return 0;
}

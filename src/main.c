#include "compile.h"
#include "util.h"
#include <time.h>
#include <string.h>

MiniOpts parse_mini_options(int argc, char **argv)
{
    MiniOpts opts = {
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
    srand(time(NULL));
    MiniOpts opts = parse_mini_options(argc, argv);
    return compile(opts);
}

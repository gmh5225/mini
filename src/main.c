#include "compile.h"
#include "util.h"
#include <time.h>
#include <string.h>

MiniOpts parse_mini_options(int argc, char **argv)
{
    MiniOpts opts = {
        .dump_flags = 0,
        .optimize_flags = DEFAULT_OPTIMIZATIONS,
        .input_filename = NULL,
        .output_filename = "a.out",
    };

    bool skip = false;
    for (int i = 1; i < argc; i++) {
        if (skip) { skip = false; continue; }

        char *arg = argv[i];

        if (strcmp(arg, "-o") == 0) {
            if (i + 1 >= argc) {
                fatal("not enough arguments for option '%s'", arg);
            }
            opts.output_filename = argv[i + 1];
            skip = true;
        }
        else if (strcmp(arg, "-dT") == 0) {
            opts.dump_flags |= DUMP_TOKENS;
        }
        else if (strcmp(arg, "-dA") == 0) {
            opts.dump_flags |= DUMP_AST;
        }
        else if (strcmp(arg, "-dSY") == 0) {
            opts.dump_flags |= DUMP_SYMBOLS;
        }
        else if (strcmp(arg, "-dIR") == 0) {
            opts.dump_flags |= DUMP_IR;
        }
        else if (strcmp(arg, "--no-fold") == 0) {
            opts.optimize_flags ^= O_FOLD_CONSTANTS;
            LOG_WARN("constant folding and common subexpression elimination disabled.");
        }
        else {
            opts.input_filename = arg;
        }
    }

    if (!opts.input_filename) {
        fatal("input file is required");
    }

    return opts;
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    MiniOpts opts = parse_mini_options(argc, argv);
    return compile(opts);
}

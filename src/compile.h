#ifndef MINI_COMPILE_H
#define MINI_COMPILE_H

#include "symbols.h"

enum
{ 
    DUMP_TOKENS = 1 << 1, 
    DUMP_AST = 1 << 2,
    DUMP_SYMBOLS = 1 << 3,
    DUMP_IR = 1 << 4,
};

typedef struct MiniOpts MiniOpts;
struct MiniOpts
{
    int dump_flags;
    char *input_filename;
    char *output_filename;
};

MiniOpts parse_mini_options(int argc, char **argv);

int compile(MiniOpts opts);

extern SymbolTable *global_scope;

#endif

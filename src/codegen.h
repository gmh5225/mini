#ifndef MINI_CODEGEN_H
#define MINI_CODEGEN_H

#include "cfa.h"

/* Available Backends */
int nasm_x86_64_generate(ControlFlowGraph *graph);

#endif

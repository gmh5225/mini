#ifndef MINIGEN_IR_H
#define MINIGEN_IR_H

#include <stddef.h>

typedef struct minigen_basic_block {
    int id;
    void *entry;
    void *exit;
} BasicBlock;

typedef struct minigen_ir {
    BasicBlock *blocks;
    size_t num_blocks;
} MinigenIR;

#endif

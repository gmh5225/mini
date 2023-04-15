#ifndef MINI_IR_H
#define MINI_IR_H

#include "parse.h"
#include "util/vector.h"

#include <stddef.h>

typedef struct BasicBlock BasicBlock;
struct BasicBlock {
    int id;
    char *tag;
    Vector predecessors;    // BasicBlock *
    Vector successors;      // BasicBlock *
    Vector statements;      // ASTNode *
};

typedef struct {
    BasicBlock *blocks;
    size_t num_blocks;
} ControlFlowGraph;

ControlFlowGraph generate_control_flow_graph(ASTNode *root);
BasicBlock *cfg_get_block(ControlFlowGraph *graph, int block_id);
BasicBlock *cfg_get_last_block(ControlFlowGraph *graph);

void translate_to_ssa(ControlFlowGraph *graph);

#endif

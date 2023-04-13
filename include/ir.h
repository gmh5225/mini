#ifndef MINI_IR_H
#define MINI_IR_H

#include "parse.h"

#include <stddef.h>

typedef struct {
    int id;
    char *tag;
    int *predecessors;
    size_t num_predecessors;
    size_t pred_capacity;
    int *successors;
    size_t num_successors;
    size_t succ_capacity;
    ASTNode **statements;
    size_t num_statements;
    size_t stmt_capacity;
} BasicBlock;

typedef struct {
    BasicBlock *blocks;
    size_t num_blocks;
} ControlFlowGraph;

ControlFlowGraph generate_control_flow_graph(ASTNode *root);
BasicBlock *cfg_get_block(ControlFlowGraph *graph, int block_id);
BasicBlock *cfg_get_last_block(ControlFlowGraph *graph);

void translate_to_ssa(ControlFlowGraph *graph);

#endif

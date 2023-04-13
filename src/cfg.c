#include "ir.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_CFG_CAPACITY 16
#define DEFAULT_BLOCK_CAPACITY 8

enum { BLOCK_PREDECESSOR, BLOCK_SUCCESSOR };

typedef struct {
    BasicBlock *curr_block;
    ControlFlowGraph *graph;
    size_t graph_capacity;
} GraphBuilder;

/* BasicBlock */
static void basic_block_init(BasicBlock *block, int id, char *tag) {
    block->id = id;
    block->tag = tag;
    block->predecessors = calloc(DEFAULT_BLOCK_CAPACITY, sizeof(int));
    block->num_predecessors = 0;
    block->pred_capacity = DEFAULT_BLOCK_CAPACITY;
    block->successors = calloc(DEFAULT_BLOCK_CAPACITY, sizeof(int));
    block->num_successors = 0;
    block->succ_capacity = DEFAULT_BLOCK_CAPACITY;
    block->statements = calloc(DEFAULT_BLOCK_CAPACITY, sizeof(ASTNode *));
    block->num_statements = 0;
    block->stmt_capacity = DEFAULT_BLOCK_CAPACITY;
}

/* ControlFlowGraph */
static void cfg_init(ControlFlowGraph *graph, size_t capacity) {
    graph->blocks = calloc(capacity, sizeof(BasicBlock));
    graph->num_blocks = 0;
}

BasicBlock *cfg_get_block(ControlFlowGraph *graph, int block_id) {
    if (block_id >= 0 && block_id < graph->num_blocks)
        return &graph->blocks[block_id];
    return NULL;
}

BasicBlock *cfg_get_last_block(ControlFlowGraph *graph) {
    return &graph->blocks[graph->num_blocks - 1];
}

/* GraphBuilder */
static void graph_builder_init(GraphBuilder *builder, ControlFlowGraph *graph) {
    builder->curr_block = NULL;
    builder->graph = graph;
    builder->graph_capacity = DEFAULT_CFG_CAPACITY;
    cfg_init(builder->graph, builder->graph_capacity);
}

static void graph_builder_add_block(GraphBuilder *builder, char *tag) {
    ControlFlowGraph *graph = builder->graph;
    if (graph->num_blocks >= builder->graph_capacity) {
        builder->graph_capacity <<= 1;
        void *tmp = realloc(graph->blocks, sizeof(BasicBlock) * builder->graph_capacity);
        graph->blocks = tmp;
    }
    
    int next_block_id = graph->num_blocks;

    BasicBlock *next_block = &graph->blocks[next_block_id];
    basic_block_init(next_block, next_block_id, tag);

    builder->curr_block = next_block;
    graph->num_blocks++;
}

static void graph_builder_add_node(GraphBuilder *builder, ASTNode *node) {
    BasicBlock *block = builder->curr_block;
    if (block->num_statements >= block->stmt_capacity) {
        block->stmt_capacity <<= 1;
        void *tmp = realloc(block->statements, sizeof(ASTNode *) * block->stmt_capacity);
        block->statements = tmp;
    }

    node->next = NULL;
    block->statements[block->num_statements++] = node;
}

static void generate_cfg(GraphBuilder *builder, ASTNode *root) {
    if (!root) return;

    ASTNode *next = root->next;
    switch (root->kind) {
        case NODE_FUNC_DECL:
            graph_builder_add_block(builder, root->func_decl.name);
            generate_cfg(builder, root->func_decl.params);
            generate_cfg(builder, root->func_decl.body);
            break;
        case NODE_VAR_DECL:
        case NODE_ASSIGN_STMT:
            graph_builder_add_node(builder, root);
            break;
        case NODE_RET_STMT:
            graph_builder_add_block(builder, builder->curr_block->tag);
            graph_builder_add_node(builder, root->ret_stmt.value);
            break;
        case NODE_COND_STMT:
            graph_builder_add_node(builder, root->cond_stmt.expr);
            graph_builder_add_block(builder, builder->curr_block->tag);
            generate_cfg(builder, root->cond_stmt.body);
            break;
        case NODE_UNARY_EXPR:
        case NODE_BINARY_EXPR:
        case NODE_LITERAL_EXPR:
        case NODE_REF_EXPR:
        default: error("cannot generate CFG from node: %d", root->kind);
    }

    generate_cfg(builder, next);
}

ControlFlowGraph generate_control_flow_graph(ASTNode *root) {
    GraphBuilder builder = {0};
    ControlFlowGraph graph = {0};
    graph_builder_init(&builder, &graph);

    graph_builder_add_block(&builder, "$entry");
    generate_cfg(&builder, root);
    graph_builder_add_block(&builder, "$exit");

    return graph;
}

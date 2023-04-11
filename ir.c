#include "mini.h"

void print_blocks(BasicBlock *block, const char *tag) {
    if (!block) return;

    printf("[BasicBlock(%d)]%s", block->id, tag ? " -> " : "\n");
    if (tag) { printf("'%s'\n", tag); }
    dump_ast(block->first);

    print_blocks(block->next, NULL);
}

static BasicBlock *basic_block_create() {
    static int unique_id = 0;
    BasicBlock *block = calloc(1, sizeof(BasicBlock));
    block->id = unique_id++;
    block->first = block->last = NULL;
    block->next = NULL;
    return block;
}

static void connect_blocks(BasicBlock *start, BasicBlock *end) {
    start->next = end;
}

static void construct(Node *root, BasicBlock *start, BasicBlock *end) {
    if (!root) return;

    switch (root->kind) {
        case NODE_ASSIGN_EXPR:
            break;
        default:
            if (!start->first) {
                start->first = root;
            }
            construct(root->next, start, end);
    }
}

BasicBlock *construct_control_flow_graph(Node *root) {
    BasicBlock *entry = basic_block_create();
    BasicBlock *exit = basic_block_create();

    construct(root, entry, exit);
    connect_blocks(entry, exit);

    return entry;
}

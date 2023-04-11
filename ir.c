#include "mini.h"

void print_blocks(basic_block *block, const char *tag) {
    if (!block) return;

    printf("[basic_block(%d)]%s", block->id, tag ? " -> " : "\n");
    if (tag) { printf("'%s'\n", tag); }
    dump_ast(block->first);

    print_blocks(block->next, NULL);
}

static basic_block *basic_block_create() {
    static int unique_id = 0;
    basic_block *block = calloc(1, sizeof(basic_block));
    block->id = unique_id++;
    block->first = block->last = NULL;
    block->next = NULL;
    return block;
}

static void connect_blocks(basic_block *start, basic_block *end) {
    start->next = end;
}

static void construct(ast_node *root, basic_block *start, basic_block *end) {
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

basic_block *construct_control_flow_graph(ast_node *root) {
    basic_block *entry = basic_block_create();
    basic_block *exit = basic_block_create();

    construct(root, entry, exit);
    connect_blocks(entry, exit);

    return entry;
}

#include "compile.h"
#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>

int compile(ASTNode *program, char *output_filename) {
    CodeBuffer output = {0};
    code_buffer_init(&output);

    BasicBlock *blocks = translate_to_ssa(program);
#ifdef DEBUG
    printf("Translated to SSA form:\n");
    BasicBlock *block = blocks;
    while (block) {
        printf("[BasicBlock %s#%d]\n", block->tag, block->id);

        SSANode *node = block->nodes;
        while (node) {
            printf(" SSANode : %s#%d\n", node->name, node->sub);
            dump_ast(node->value, 2);
            node = node->next;
        }

        block = block->next;
    }
#endif

    code_buffer_write_to_file(&output, output_filename);
    code_buffer_free(&output);
    return 0;
}

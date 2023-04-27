#include "compile.h"
#include "lex.h"
#include "ir.h"
#include "parse.h"
#include "types.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable *global_scope = NULL;

static void initialize_compiler_context()
{
    global_scope = symbol_table_create("__GLOBAL__");
    if (!global_scope) {
        fatal("couldn't allocate symbol table for global_scope!");
    }

    // Add supported primitive types to global scope
    for (TypeKind kind = TYPE_VOID; kind <= TYPE_BOOL; kind++) {
        Type primitive = primitive_types[kind];
        Symbol *primitive_sym = symbol_table_insert(global_scope, primitive.name, SYMBOL_TYPE);
        primitive_sym->type = primitive;
    }
}

int compile(MiniOpts opts)
{
    int status = 0;
    FILE *file = fopen(opts.input_filename, "rb");
    if (!file) {
        fatal("couldn't open file `%s`", opts.input_filename);
    }

    initialize_compiler_context();

    // Lexical Analysis
    Vector tokens = lex(file);
    if (opts.dump_flags & DUMP_TOKENS) {
        for (size_t i = 0; i < tokens.size; i++) {
            Token *token = (Token *)vector_get(&tokens, i);
            if (token->kind == TOKEN_EOF) break;
            printf("%s\n", token_as_str(token->kind));
        }
    }
    fclose(file);

    // Semantic Analysis
    ASTNode *ast = parse(tokens);
    if (opts.dump_flags & DUMP_AST)
        dump_ast(ast, 0);

    if (opts.dump_flags & DUMP_SYMBOLS)
        symbol_table_dump(global_scope, 0);

    // Optimization: Constant Folding
    if (opts.optimize_flags & O_FOLD_CONSTANTS)
        fold_constants(ast);

    // IR Translation
    Symbol *entry_point = symbol_table_lookup(global_scope, "main");
    Program program = emit_ir(entry_point->node);
    if (opts.dump_flags & DUMP_IR) {
        BasicBlock *block = program.blocks;
        while (block) {
            printf("[BasicBlock %s#%d] (%ld predecessors, %ld successors, %ld instructions)\n",
                    block->tag, block->id,
                    block->predecessors.size,
                    block->successors.size,
                    block->instructions.size);

            for (size_t i = 0; i < block->instructions.size; i++) {
                Instruction *inst = (Instruction *)vector_get(&block->instructions, i);
                dump_instruction(inst);
            }

            block = block->next;
        }
    }

    // Iterate top-level of AST and print warnings for unused nodes
    ASTNode *iter = ast;
    while (iter) {
        if (!iter->visited) {
            switch (iter->kind) {
                case NODE_FUNC_DECL:
                    LOG_WARN("unused function %s at line %d, col %d",
                            iter->func_decl.name, iter->line, iter->col);
                    break;
                case NODE_VAR_DECL:
                    LOG_WARN("unused variable %s at line %d, col %d", 
                            iter->var_decl.name, iter->line, iter->col);
                    break;
            }
        }

        iter = iter->next;
    }

    return status;
}

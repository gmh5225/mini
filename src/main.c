#include "cfa.h"
#include "compile.h"
#include "codegen.h"
#include "lex.h"
#include "parse.h"
#include "util.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct
{
    int dump_flags;
    int optimize_flags;
    char *input_filename;
    char *output_filename;
} MiniOpts;

MiniOpts parse_mini_options(int argc, char **argv)
{
  MiniOpts opts = {
    .dump_flags = 0,
    .optimize_flags = DEFAULT_OPTIMIZATIONS,
    .input_filename = NULL,
    .output_filename = "a.out",
  };

  bool skip = false;
  for (int i = 1; i < argc; i++) {
    if (skip) { skip = false; continue; }

    char *arg = argv[i];

    if (strcmp(arg, "-o") == 0) {
      if (i + 1 >= argc) {
        fatal("not enough arguments for option '%s'", arg);
      }
      opts.output_filename = argv[i + 1];
      skip = true;
    }
    else if (strcmp(arg, "-dT") == 0) {
      opts.dump_flags |= DUMP_TOKENS;
    }
    else if (strcmp(arg, "-dA") == 0) {
      opts.dump_flags |= DUMP_AST;
    }
    else if (strcmp(arg, "-dSY") == 0) {
      opts.dump_flags |= DUMP_SYMBOLS;
    }
    else if (strcmp(arg, "-dIR") == 0) {
      opts.dump_flags |= DUMP_IR;
    }
    else if (strcmp(arg, "--no-fold") == 0) {
      opts.optimize_flags ^= O_FOLD_CONSTANTS;
      LOG_WARN("constant folding and common subexpression elimination disabled.");
    }
    else {
      opts.input_filename = arg;
    }
  }

  if (!opts.input_filename)
    fatal("input file is required");

  return opts;
}

int main(int argc, char **argv)
{
  srand(time(NULL));
  MiniOpts opts = parse_mini_options(argc, argv);

  FILE *file = fopen(opts.input_filename, "rb");
  if (!file) {
    fatal("couldn't open file `%s`", opts.input_filename);
  }

  compiler_context_init();

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
  Node *ast = parse(tokens);
  if (opts.dump_flags & DUMP_AST)
    dump_ast(ast, 0);

  if (opts.dump_flags & DUMP_SYMBOLS)
    symbol_table_dump(ctx->global_scope, 0);

  // Optimization: Constant Folding
  if (opts.optimize_flags & O_FOLD_CONSTANTS)
    fold_constants(ast);

  // IR Translation
  Symbol *entry_point = symbol_table_lookup(ctx->global_scope, "main");
  ControlFlowGraph program = construct_cfg(entry_point->node);
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
  Node *iter = ast;
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

  nasm_x86_64_generate(&program);

  compiler_context_free();

  return 0;
}

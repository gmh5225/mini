#ifndef MINI_COMPILE_H
#define MINI_COMPILE_H

#include "symbols.h"

enum
{ 
  DUMP_TOKENS = 1 << 1, 
  DUMP_AST = 1 << 2,
  DUMP_SYMBOLS = 1 << 3,
  DUMP_IR = 1 << 4,
};

enum
{
  O_FOLD_CONSTANTS = 1 << 1,
};

#define DEFAULT_OPTIMIZATIONS \
  O_FOLD_CONSTANTS

typedef struct
{
  SymbolTable *global_scope;
  TypeID registered_types;
} CompilerContext;

void compiler_context_init();
void compiler_context_free();

extern CompilerContext *ctx;

#endif

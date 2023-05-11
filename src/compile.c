#include "compile.h"
#include "lex.h"
#include "cfa.h"
#include "parse.h"
#include "types.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CompilerContext *ctx = NULL;

void compiler_context_init()
{
  ctx = calloc(1, sizeof(CompilerContext));

  ctx->global_scope = symbol_table_create("__GLOBAL__");
  if (!ctx->global_scope)
    fatal("couldn't allocate symbol table for global_scope!");

  TypeKind kind;
  // Add supported primitive types to global scope
  for (kind = TYPE_VOID; kind <= TYPE_BOOL; kind++) {
    Type primitive = primitive_types[kind];
    Symbol *primitive_sym = symbol_table_insert(ctx->global_scope, primitive.name, SYMBOL_TYPE);
    primitive_sym->type = primitive;
  }
  ctx->registered_types = kind;
}

void compiler_context_free()
{
  // TODO: add function to free symbol table
  free(ctx);
}

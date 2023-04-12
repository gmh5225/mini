#ifndef MINIGEN_BUILDER_H
#define MINIGEN_BUILDER_H

#include "minigen/constructs.h"

typedef struct minigen_ir_builder IRBuilder;

IRBuilder minigen_ir_builder_create();
void minigen_ir_builder_finalize(IRBuilder *builder);
void minigen_ir_builder_free(IRBuilder *builder);

#endif

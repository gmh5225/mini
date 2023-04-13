#ifndef MINIGEN_CODEGEN_H
#define MINIGEN_CODEGEN_H

#include "parse.h"

#include <stddef.h>
#include <stdint.h>

typedef struct SSANode SSANode;
typedef struct BasicBlock BasicBlock;

struct SSANode {
    int sub;
    char *name;
    ASTNode *value;
    SSANode *next;
};

struct BasicBlock {
    int id;
    char *tag;
    SSANode *nodes;
    BasicBlock *next;
};

BasicBlock *translate_to_ssa(ASTNode *root);

#define DEFAULT_TARGET_CODE_CAPACITY 2048

typedef struct {
    char *buffer;
    size_t code_length;
    size_t code_capacity;
} CodeBuffer;

#define CB_WRITE(buf, bytes, len) \
    code_buffer_write_bytes(buf, bytes, len)

void code_buffer_init(CodeBuffer *code);
void code_buffer_write_bytes(CodeBuffer *code, char *bytes, size_t length);
void code_buffer_write_to_file(CodeBuffer *code, char *filename);
void code_buffer_free(CodeBuffer *code);

/* Backend entry points */
void nasm_x86_64_generate(CodeBuffer *code, BasicBlock *program);

#endif

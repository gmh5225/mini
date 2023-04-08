#ifndef MINI_SYMBOL_TABLE_H
#define MINI_SYMBOL_TABLE_H

#include "ast.h"

#include <stddef.h>

#define SYMBOL_TABLE_SIZE            256
#define SYMBOL_TABLE_CHILD_CAPACITY  4

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
} symbol_type;

const char *symbol_as_str(symbol_type type);

typedef struct symbol_info {
    char *name;
    symbol_type type;
    int size;
    int offset;
    bool is_constant;
    bool is_initialized;
    struct symbol_info *next;
} symbol_info;

typedef struct symbol_table {
    char *scope_name;
    symbol_info *symbols[SYMBOL_TABLE_SIZE];
    struct symbol_table *parent;
    struct symbol_table **children;
    size_t num_children;
    size_t child_capacity;
} symbol_table;

symbol_table *symbol_table_create(char *name);
symbol_info *symbol_table_insert(symbol_table *table, char *symbol_name, symbol_type type);
symbol_info *symbol_table_lookup(symbol_table *table, char *symbol_name);
void symbol_table_add_child(symbol_table *parent, symbol_table *child);
void symbol_table_dump(symbol_table *table);

extern symbol_table *global_scope;

#endif

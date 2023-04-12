#ifndef MINI_SYMBOLS_H
#define MINI_SYMBOLS_H

#include "types.h"

typedef enum {
    SYMBOL_UNKNOWN,
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
} SymbolKind;

extern const char *symbol_strings[];
const char *symbol_as_str(SymbolKind kind);

typedef struct Symbol Symbol;
struct Symbol {
    SymbolKind kind;
    Symbol *next;
    Type type;
    char *name;
    int align;
    int offset;
    bool is_constant;
    bool is_initialized;
};

#define SYMBOL_TABLE_SIZE 256

typedef struct SymbolTable SymbolTable;
struct SymbolTable {
    char *name;
    Symbol *symbols[SYMBOL_TABLE_SIZE];
    SymbolTable *parent;
    SymbolTable *child;
    SymbolTable *next;
};

SymbolTable *symbol_table_create(char *table_name);
Symbol *symbol_table_insert(SymbolTable *table, char *symbol_name, SymbolKind kind);
Symbol *symbol_table_lookup(SymbolTable *table, char *symbol_name);
void symbol_table_add_child(SymbolTable *parent, SymbolTable *child);
void symbol_table_dump(SymbolTable *table);

extern SymbolTable *global_scope;
void init_global_scope();

#endif

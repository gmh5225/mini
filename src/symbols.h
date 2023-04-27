#ifndef MINI_SYMBOLS_H
#define MINI_SYMBOLS_H

#include "parse.h"
#include "types.h"

typedef enum SymbolKind SymbolKind;
typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;

enum SymbolKind
{
    SYMBOL_UNKNOWN,
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
};

extern const char *symbol_strings[];
const char *symbol_as_str(SymbolKind kind);

struct Symbol
{
    SymbolKind kind;
    char *name;
    bool is_constant;
    bool is_initialized;
    Type type;
    ASTNode *node;
    Symbol *next;
};

#define SYMBOL_TABLE_SIZE 256

struct SymbolTable
{
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
void symbol_table_dump(SymbolTable *table, int level);

#endif

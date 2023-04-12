#include "symbols.h"
#include "types.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

SymbolTable *global_scope = NULL;

const char *symbol_strings[] = {
    [SYMBOL_UNKNOWN] = "[UNKNOWN SYMBOL]",
    [SYMBOL_VARIABLE] = "[VARIABLE]",
    [SYMBOL_FUNCTION] = "[FUNCTION]",
};

const char *symbol_as_str(SymbolKind type) {
    return symbol_strings[type];
}

void init_global_scope() {
    global_scope = symbol_table_create("__GLOBAL__");

    // Add supported primitive types to global scope
    for (TypeKind kind = TYPE_VOID; kind <= TYPE_BOOL; kind++) {
        Type primitive = primitive_types[kind];
        Symbol *p_type_sym = symbol_table_insert(global_scope, primitive.name, SYMBOL_TYPE);
        p_type_sym->type = primitive;
    }
}

uint64_t hash(char *s) {
    uint64_t hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static Symbol *symbol_info_create(void) {
    Symbol *s = malloc(sizeof(Symbol));
    s->kind = SYMBOL_UNKNOWN;
    s->next = NULL;
    s->type = (Type){.kind = TYPE_UNKNOWN};
    s->name = NULL;
    s->align = -1;
    s->offset = -1;
    s->is_constant = false;
    s->is_initialized = false;
    return s;
}

SymbolTable *symbol_table_create(char *name) {
    size_t length = strlen(name);
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->name = malloc(sizeof(char) * (length + 1));
    memcpy(table->name, name, length);
    table->name[length] = 0;

    // Preallocate all symbol_info entries in hash table
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        table->symbols[i] = symbol_info_create();
    }

    table->parent = NULL;
    table->child = NULL;
    table->next = NULL;

    return table;
}

Symbol *symbol_table_insert(SymbolTable *table, char *symbol_name, SymbolKind kind) {
    if (!table) return NULL;

    // Check if the Symbol already exists in one of the current table or parent tables.
    // If the Symbol is a function, ignore this check as we will still add it to its own
    // scope to allow for recursion.
    Symbol *exists = NULL;
    if ((exists = symbol_table_lookup(table, symbol_name))) {
        return NULL;
    }

    uint64_t index = hash(symbol_name) % SYMBOL_TABLE_SIZE;
    Symbol *info = table->symbols[index];

    // If a symbol_info already exists at index AND is contains valid Symbol information,
    // deal with the hash collision by appending to the linked list
    if (info && info->name) {
        Symbol *new_info = symbol_info_create();
        new_info->name = symbol_name;
        new_info->kind = kind;
        new_info->next = info;
        return new_info;
    }

    // Otherwise, edit the data at index in place and return it.
    info->name = symbol_name;
    info->kind = kind;
    info->next = NULL;
    return info;
}

Symbol *symbol_table_lookup(SymbolTable *table, char *symbol_name) {
    if (!table) return NULL;

    uint64_t index = hash(symbol_name) % SYMBOL_TABLE_SIZE;
    Symbol *info = table->symbols[index];

    while (info && info->name) {
        if (memcmp(info->name, symbol_name, strlen(symbol_name)) == 0) {
            return info;
        }
        info = info->next;
    }

    // If we couldn't find the Symbol in the current scope, do lookup in the parent
    if (table->parent) {
        return symbol_table_lookup(table->parent, symbol_name);
    }

    return NULL;
}

void symbol_table_add_child(SymbolTable *parent, SymbolTable *child) {
    parent->child = child;
    child->parent = parent;
}

void symbol_table_dump_impl(SymbolTable *table, int level) {
    for (int i = 0; i < level; i++) {
        printf("    ");
    }

    printf("Scope: %s\n", table->name);
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        Symbol *info = table->symbols[i];
        while (info && info->name) {
            for (int i = 0; i < level; i++) {
                printf("    ");
            }
            printf("  name: %s, type: %s", 
                    info->name, symbol_as_str(info->kind));
            printf(info->next ? "  ->  " : "\n");
            info = info->next;
        }
    }

    if (table->child) {
        symbol_table_dump_impl(table->child, level + 1);
    }

    if (table->next) {
        symbol_table_dump_impl(table->next, level);
    }
}

void symbol_table_dump(SymbolTable *table) {
    symbol_table_dump_impl(table, 0);
}

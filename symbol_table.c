#include "symbol_table.h"
#include "util.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

symbol_table *global_scope = NULL;

uint64_t hash(char *s) {
    uint64_t hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static symbol_info *symbol_info_create(void) {
    symbol_info *symbol = malloc(sizeof(symbol_info));
    symbol->name = NULL;
    symbol->type = 0;
    symbol->offset = -1;
    symbol->is_constant = false;
    symbol->is_initialized = false;
    symbol->next = NULL;
    return symbol;
}

symbol_table *symbol_table_create(char *name) {
    size_t length = strlen(name);
    symbol_table *table = malloc(sizeof(symbol_table));
    table->scope_name = malloc(sizeof(char) * (length + 1));
    memcpy(table->scope_name, name, length);
    table->scope_name[length] = 0;

    // Preallocate all symbol_info entries in hash table
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        table->symbols[i] = symbol_info_create();
    }

    table->parent = NULL;
    table->children = calloc(SYMBOL_TABLE_CHILD_CAPACITY, sizeof(symbol_table));
    table->num_children = 0;
    table->child_capacity = SYMBOL_TABLE_CHILD_CAPACITY;

    return table;
}

symbol_info *symbol_table_insert(symbol_table *table, char *symbol_name, ast_node_type type) {
    if (!table) return NULL;

    // Check if the symbol already exists in one of the current table or parent tables.
    // If the symbol is a function, ignore this check as we will still add it to its own
    // scope to allow for recursion.
    symbol_info *exists = NULL;
    if ((exists = symbol_table_lookup(table, symbol_name)) && type != NODE_FUNCTION) {
        return NULL;
    }

    uint64_t index = hash(symbol_name) % SYMBOL_TABLE_SIZE;
    symbol_info *info = table->symbols[index];

    // If a symbol_info already exists at index AND is contains valid symbol information,
    // deal with the hash collision by appending to the linked list
    if (info && info->name) {
        symbol_info *new_info = symbol_info_create();
        new_info->name = symbol_name;
        new_info->type = type;
        new_info->next = info;
        return new_info;
    }

    // Otherwise, edit the data at index in place and return it.
    info->name = symbol_name;
    info->type = type;
    info->next = NULL;
    return info;
}

symbol_info *symbol_table_lookup(symbol_table *table, char *symbol_name) {
    if (!table) return NULL;

    uint64_t index = hash(symbol_name) % SYMBOL_TABLE_SIZE;
    symbol_info *info = table->symbols[index];

    while (info && info->name) {
        if (memcmp(info->name, symbol_name, strlen(symbol_name)) == 0) {
            return info;
        }
        info = info->next;
    }

    // If we couldn't find the symbol in the current scope, search in the parent of the current
    // table first, and then finally the __GLOBAL__ scope
    if (table != global_scope && table->parent) {
        return symbol_table_lookup(table->parent, symbol_name);
    }

    return NULL;
}

void symbol_table_add_child(symbol_table *parent, symbol_table *child) {
    if (parent->num_children >= parent->child_capacity) {
        parent->child_capacity <<= 1;
        void *tmp = realloc(parent->children, sizeof(symbol_table) * parent->child_capacity);
        parent->children = tmp;
    }
    child->parent = parent;
    parent->children[parent->num_children++] = child;
}

void symbol_table_dump_impl(symbol_table *table, int level) {
    for (int i = 0; i < level; i++) {
        printf("    ");
    }

    printf("Scope: %s (%ld subscopes)\n", table->scope_name, table->num_children);
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        symbol_info *info = table->symbols[i];
        while (info && info->name) {
            for (int i = 0; i < level; i++) {
                printf("    ");
            }
            printf("{ name: %s, type: %s }", info->name, node_as_str(info->type));
            if (info->next) {
                printf("  ->  ");
            } else {
                printf("\n");
            }

            info = info->next;
        }
    }

    for (size_t i = 0; i < table->num_children; i++) {
        symbol_table_dump_impl(table->children[i], level + 1); 
    }
}

void symbol_table_dump(symbol_table *table) {
    symbol_table_dump_impl(table, 0);
}

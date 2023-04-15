#ifndef MINI_TABLE_H
#define MINI_TABLE_H

#define TABLE_SIZE 1024

#include <stddef.h>

typedef struct TableEntry TableEntry;

typedef struct {
    TableEntry *entries[TABLE_SIZE];
} Table;

Table *table_new();
void table_insert(Table *table, const char *key, void *value);
void *table_lookup(Table *table, const char *key);
void table_clear(Table *table);
void table_free(Table *table);

#endif

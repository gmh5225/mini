#include "util/table.h"
#include "util/util.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct TableEntry {
    char *key;
    void *value;
    TableEntry *next;
};

Table *table_new() {
    Table *table = calloc(1, sizeof(Table));
    return table;
}

void table_insert(Table *table, const char *key, void *value) {
    uint64_t index = hash(key) % TABLE_SIZE;
    TableEntry *entry = table->entries[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }

    size_t key_length = strlen(key);
    TableEntry *new_entry = calloc(1, sizeof(TableEntry));
    new_entry->key = calloc(key_length, sizeof(char));
    memcpy(new_entry->key, key, key_length);
    new_entry->value = value;
    new_entry->next = table->entries[index];
    table->entries[index] = new_entry;
}

void *table_lookup(Table *table, const char *key) {
    uint64_t index = hash(key) % TABLE_SIZE;
    TableEntry *entry = table->entries[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0)
            return entry->value;
        entry = entry->next;
    }

    return NULL;
}

void table_clear(Table *table) {
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        TableEntry *entry = table->entries[i];
        while (entry) {
            TableEntry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
}

void table_free(Table *table) {
    if (table) {
        table_clear(table);
        free(table);
    }
}

#include "main.h"
#include "symbol-table.h"

#include <stdio.h>

#include <string>

static
size_t getIndex(SymbolTable *table, string id) {
    /* hash algorithm from http://www.cse.yorku.ca/~oz/hash.html */

    char c;
    const char *cur = id.data();
    size_t hash = 5381;
    while ((c = *cur++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    assert(table->count < table->size);

    /* either find the entry with the id or an empty slot */

    SymbolEntry *data = table->data;
    size_t index = hash & (table->size - 1); /* truncate to range */
    while (!data[index].id.empty() && data[index].id != id) {
        index = (index + 1) & (table->size - 1);
    }

    return index;
}

void initSymbolTable(SymbolTable *table, string name, size_t size) {
    assert(size && (size & (size - 1)) == 0); /* check for power of 2 */

    *table = (SymbolTable){
        .name = name,
        .data = new SymbolEntry[size],
        .order = new size_t[size],
        .count = 0,
        .size = size,
    };
}

void deinitSymbolTable(SymbolTable *table) {
    delete[] table->data;
    delete[] table->order;
}

static
bool _addSymbol(SymbolTable *table, string id, string type, string value) {
    assert(table->count < table->size);

    size_t index = getIndex(table, id);
    table->order[table->count++] = index;

    SymbolEntry *entry = table->data + index;

    if (entry->id.empty()) {
        *entry = (SymbolEntry){
            .id = id,
            .type = type,
            .value = value,
        };

        return true;
    }
    else {
        return false;
    }
}

bool addSymbol(SymbolTable *table, string id, string type, string value) {
    if (4*table->count > 3*table->size) {
        /* keep table at at most 3/4 capacity */

        SymbolTable oldTable = *table;
        initSymbolTable(table, oldTable.name, oldTable.size << 1);

        for (size_t i = 0; i < oldTable.count; ++i) {
            size_t index = oldTable.order[i];
            SymbolEntry entry = oldTable.data[index];

            assert(!entry.id.empty());

            if (!_addSymbol(table, entry.id, entry.type, entry.value)) {
                InvalidCodePath;
            }
        }

        deinitSymbolTable(&oldTable);
    }

    return _addSymbol(table, id, type, value);
}

SymbolEntry getSymbol(SymbolTable *table, string id) {
    size_t index = getIndex(table, id);
    SymbolEntry entry = table->data[index];
    return entry;
}

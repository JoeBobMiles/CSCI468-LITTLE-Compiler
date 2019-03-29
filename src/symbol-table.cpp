#include "main.h"
#include "symbol-table.h"

#include "strings.h"

#include <cstdio>

static
size_t getIndex(SymbolTable *table, cchar *id) {
    /* hash algorithm from http://www.cse.yorku.ca/~oz/hash.html */

    char c;
    cchar *cur = id;
    size_t hash = 5381;
    while ((c = *cur++)) {
        hash = (33 * hash) + c;
    }

    assert(table->count < table->size);

    /* either find the entry with the id or an empty slot */

    SymbolEntry *data = table->data;
    size_t index = hash & (table->size - 1); /* truncate to range */
    while (data[index].id && !stringsAreEqual(data[index].id, id)) {
        index = (index + 1) & (table->size - 1);
    }

    return index;
}

void initSymbolTable(SymbolTable *table, cchar *name, size_t size) {
    assert(size && (size & (size - 1)) == 0); /* check for power of 2 */

    *table = (SymbolTable){
        .name = name,
        .data = (SymbolEntry *)malloc(size * sizeof *table->data),
        .order = (size_t *)malloc(size * sizeof *table->order),
        .count = 0,
        .size = size,
    };

    zeroMemory((char *)table->data, size * sizeof *table->data);
}

void deinitSymbolTable(SymbolTable *table) {
    free((void *)table->data);
    free((void *)table->order);
}

static
bool _addSymbol(SymbolTable *table, cchar *id, cchar *type, cchar *value) {
    assert(table->count < table->size);

    size_t index = getIndex(table, id);
    table->order[table->count++] = index;

    SymbolEntry *entry = table->data + index;

    if (!entry->id) {
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

bool addSymbol(SymbolTable *table, cchar *id, cchar *type, cchar *value) {
    if (4*table->count > 3*table->size) {
        /* keep table at at most 3/4 capacity */

        SymbolTable oldTable = *table;
        initSymbolTable(table, oldTable.name, oldTable.size << 1);

        for (size_t i = 0; i < oldTable.count; ++i) {
            size_t index = oldTable.order[i];
            SymbolEntry entry = oldTable.data[index];

            assert(entry.id);

            if (!_addSymbol(table, entry.id, entry.type, entry.value)) {
                InvalidCodePath;
            }
        }

        deinitSymbolTable(&oldTable);
    }

    return _addSymbol(table, id, type, value);
}

SymbolEntry getSymbol(SymbolTable *table, cchar *id) {
    size_t index = getIndex(table, id);
    SymbolEntry entry = table->data[index];
    return entry;
}

#include "main.h"
#include "symbol-table.h"

#include "strings.h"

#include <cstdio>

static
u32 getIndex(SymbolTable *table, cchar *id) {
    /* hash algorithm from http://www.cse.yorku.ca/~oz/hash.html */

    char c;
    cchar *cur = id;
    u32 hash = 5381;
    while ((c = *cur++)) {
        hash = (33 * hash) + c;
    }

    assert(table->count < table->size);

    /* either find the entry with the id or an empty slot */

    SymbolEntry *data = table->data;
    u32 index = hash & (table->size - 1); /* truncate to range */
    while (data[index].id && !stringsAreEqual(data[index].id, id)) {
        index = (index + 1) & (table->size - 1);
    }

    return index;
}

void initSymbolTable(SymbolTable *table, cchar *name, u32 size) {
    assert(size && (size & (size - 1)) == 0); /* check for power of 2 */

    *table = (SymbolTable){
        .name = name,
        .count = 0,
        .size = size,
        .data = (SymbolEntry *)malloc(size * sizeof *table->data),
        .order = (u32 *)malloc(size * sizeof *table->order),
    };

    zeroMemory((char *)table->data, size * sizeof *table->data);
}

void deinitSymbolTable(SymbolTable *table) {
    free((void *)table->data);
    free((void *)table->order);
}

static
SymbolEntry *_addSymbol(SymbolTable *table, SymbolEntry *symbol) {
    assert(table->count < table->size);

    u32 index = getIndex(table, symbol->id);
    table->order[table->count++] = index;

    SymbolEntry *entry = table->data + index;

    if (!entry->id) {
        entry->id = symbol->id;
        entry->symbolType = symbol->symbolType;
        entry->logicalType = symbol->logicalType;
        entry->value = symbol->value;
        entry->root = symbol->root;

        return entry;
    }
    else {
        return 0;
    }
}

static
SymbolEntry *addSymbol(SymbolTable *table, SymbolEntry *symbol) {
    if (4*table->count > 3*table->size) {
        /* keep table at at most 3/4 capacity */

        SymbolTable oldTable = *table;
        initSymbolTable(table, oldTable.name, oldTable.size << 1);

        for (u32 i = 0; i < oldTable.count; ++i) {
            u32 index = oldTable.order[i];
            SymbolEntry *entry = oldTable.data + index;

            assert(entry->id);

            if (!_addSymbol(table, entry)) {
                InvalidCodePath;
            }
        }

        deinitSymbolTable(&oldTable);
    }

    return _addSymbol(table, symbol);
}

SymbolEntry *addVar(SymbolTable *table, cchar *id, cchar type, cchar *value) {
    SymbolEntry symbol = {
        .id = id,
        .symbolType = 'v',
        .logicalType = type,
        .value = value,
        .root = 0,
    };

    return addSymbol(table, &symbol);
}

SymbolEntry *addParam(SymbolTable *table, cchar *id, cchar type) {
    SymbolEntry symbol = {
        .id = id,
        .symbolType = 'p',
        .logicalType = type,
        .value = 0,
        .root = 0,
    };

    return addSymbol(table, &symbol);
}

SymbolEntry *addFunc(SymbolTable *table, cchar *id, cchar returnType, cchar *paramTypes, AstRoot *root) {
    SymbolEntry symbol = {
        .id = id,
        .symbolType = 'f',
        .logicalType = returnType,
        .value = paramTypes,
        .root = root,
    };

    return addSymbol(table, &symbol);
}

SymbolEntry *getSymbol(SymbolTable *table, cchar *id) {
    u32 index = getIndex(table, id);
    SymbolEntry *entry = table->data + index;

    return entry->id? entry: 0;
}

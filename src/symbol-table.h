#ifndef symbol_table_h
#define symbol_table_h

#include "main.h"

struct SymbolEntry {
    cchar *id;
    cchar *type;
    cchar *value;
};

struct SymbolTable {
    cchar *name;
    SymbolEntry *data;
    size_t *order;
    size_t count;
    size_t size; /* Must be a power of 2 */
};

void initSymbolTable(SymbolTable *table, cchar *name, size_t size);
void deinitSymbolTable(SymbolTable *table);

bool addSymbol(SymbolTable *table, cchar *id, cchar *type, cchar *value = 0);
SymbolEntry getSymbol(SymbolTable *table, cchar *id);

#endif

#ifndef symbol_table_h
#define symbol_table_h

#include <string>

using namespace std;

struct SymbolEntry {
    string id;
    string type;
    string value;
};

struct SymbolTable {
    string name;
    SymbolEntry *data;
    size_t *order;
    size_t count;
    size_t size; /* Must be a power of 2 */
};

void initSymbolTable(SymbolTable *table, string name, size_t size);
void deinitSymbolTable(SymbolTable *table);

bool addSymbol(SymbolTable *table, string id, string type, string value = "");
SymbolEntry getSymbol(SymbolTable *table, string id);

#endif

#ifndef symbol_table_h
#define symbol_table_h

#include "main.h"

struct AstRoot;

struct SymbolEntry {
    cchar *id;
    char symbolType;  /* function vs var vs param*/
    char logicalType; /* int, real, void, string */
    cchar *value;     /* string value or function parameter type list */
    AstRoot *root;
};

struct SymbolTable {
    cchar *name;
    SymbolEntry *data;
    size_t *order;
    size_t count;
    size_t size; /* Must be a power of 2 */
};

#include "ast.h" /* NOTE: this MUST be below the definition of SymbolTable */

void initSymbolTable(SymbolTable *table, cchar *name, size_t size);
void deinitSymbolTable(SymbolTable *table);

SymbolEntry *addVar(SymbolTable *table, cchar *id, cchar type, cchar *value);
SymbolEntry *addFunc(SymbolTable *table, cchar *id, cchar returnType, cchar *paramTypes, AstRoot *root);

SymbolEntry getSymbol(SymbolTable *table, cchar *id);

#endif

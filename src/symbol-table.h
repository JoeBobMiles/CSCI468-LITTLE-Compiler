#ifndef symbol_table_h
#define symbol_table_h

#include "main.h"

struct AstRoot;

struct SymbolEntry {
    cchar *id;
    char symbolType;  /* function vs var vs param*/
    char logicalType; /* int, real, void, string */
    /* we have 6B of padding here */
    cchar *value;     /* string value or function parameter type list */
    AstRoot *root;
};

struct SymbolTable {
    cchar *name;
    u32 count;
    u32 size; /* Must be a power of 2 */
    SymbolEntry *data;
    u32 *order;
};

#include "ast.h" /* NOTE: this MUST be below the definition of SymbolTable */

void initSymbolTable(SymbolTable *table, cchar *name, u32 size);
void deinitSymbolTable(SymbolTable *table);

SymbolEntry *addVar(SymbolTable *table, cchar *id, cchar type, cchar *value);
SymbolEntry *addParam(SymbolTable *table, cchar *id, cchar type);
SymbolEntry *addFunc(SymbolTable *table, cchar *id, cchar returnType, cchar *paramTypes, AstRoot *root);

SymbolEntry *getSymbol(SymbolTable *table, cchar *id);

#endif

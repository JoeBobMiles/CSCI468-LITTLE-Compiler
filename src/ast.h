#ifndef ast_h
#define ast_h

#include "main.h"

struct Program;
struct AstRoot;
struct AstStatement;

#include "symbol-table.h"

#define MAX_TABLES 32 /* 32 tables should be enough for anyone */

struct AstStatement {
    /* reserved for the future */
    u32 Reserved;
};

struct AstRoot {
    cchar *id;
    SymbolTable *symbols;
    AstStatement *firstStatement;
};

struct Program {
    size_t listCount;
    size_t stackHead;
    cchar *firstError;

    /* Master list of all tables */
    SymbolTable tableList[MAX_TABLES];

    /* stack representing nesting, tables duplicated in tableList */
    SymbolTable *tableStack[MAX_TABLES];
};

#endif
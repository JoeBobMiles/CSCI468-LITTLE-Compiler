#ifndef ast_h
#define ast_h

#include "main.h"

#define MAX_TABLES 32 /* 32 tables should be enough for anyone */

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

#ifndef ast_h
#define ast_h

#include "main.h"

struct Program;
struct AstRoot;
struct AstStatement;

#include "symbol-table.h"

#define MAX_TABLES 32 /* 32 tables should be enough for anyone */

enum ExprType {
    EXPR_Null = 0,

    /* These indicate an AstBinaryOp */

    EXPR_Addition       = '+',
    EXPR_Subtraction    = '-',
    EXPR_Multiplication = '*',
    EXPR_Division       = '/',

    EXPR_LessThan           = '<',
    EXPR_GreaterThan        = '>',
    EXPR_Equal              = '=',
    EXPR_NotEqual           = '!',
    EXPR_LessThanOrEqual    = 'l',
    EXPR_GreaterThanOrEqual = 'g',

    /* These indicate an AstTerminal */

    EXPR_IntLiteral    = 'I',
    EXPR_FloatLiteral  = 'F',
    EXPR_StringLiteral = 'S',
    EXPR_Symbol        = 's',

    /* These indicate an AstFunctionCall */

    EXPR_Function     = 'f',
};

struct AstExpr {
    /* NOTE:
     *
     * This type serves as a header for other types. This is how you do
     * quasi-inheritance in C. It's glorious, if a bit janky.
     */
    ExprType type;
    u32 tempNumber; /* non-terminals get a temp number. */

    /* TODO: should this be a discriminated union instead? The sub-types are
     * of pretty similar size, after all. It would look something like this:
     * union {
     *     struct { ... } binOp;
     *     struct { ... } terminal;
     *     struct { ... } functionCall;
     * } as;
     */
};

struct AstBinaryOp {
    AstExpr header; /* must be the first member */

    AstExpr *leftChild;
    AstExpr *rightChild;
};

struct AstTerminal {
    AstExpr header; /* must be the first member */

    AstExpr *nextParam; /* TODO: this? */
    cchar *text;
};

struct AstFunctionCall {
    AstExpr header; /* must be the first member */

    cchar *functionName;
    AstExpr *firstParam; /* TODO: this? */
};



enum StatementType {
    STATEMENT_Null = 0,

    STATEMENT_Root, /* for if/else and while statements */

    /* base statements */

    STATEMENT_Assign,
    STATEMENT_Read,
    STATEMENT_Write,
    STATEMENT_Return,
};

struct AstStatement {
    AstStatement *nextStatement;
    StatementType type;
};

enum RootType {
    ROOT_Null = 0,

    ROOT_Global,
    ROOT_Function,
    ROOT_If,
    ROOT_Else,
    ROOT_While,
};

struct AstRoot {
    RootType type;
    SymbolTable *symbols;
    AstStatement *firstStatement;
};

struct Program {
    AstRoot root;
    cchar *firstError; /* TODO: is this worth it? */

    u32 blockCount;

    size_t listCount;
    size_t stackHead;

    /* Master list of all tables */
    SymbolTable tableList[MAX_TABLES];

    /* stack representing nesting, tables duplicated in tableList */
    SymbolTable *tableStack[MAX_TABLES];
};

#endif

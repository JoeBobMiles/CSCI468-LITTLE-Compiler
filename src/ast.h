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
    EXPR_Symbol        = 's',    /* the only L-value */

    /* These indicate an AstFunctionCall */

    EXPR_Function     = 'f',
};

struct AstExpr {
    ExprType type;
    u32 tempNumber; /* non-terminals get a temp number. */

    union {
        struct {
            AstExpr *leftChild;
            AstExpr *rightChild;
        } asBinaryOp;
        struct {
            AstExpr *nextParam; /* TODO: this? */
            cchar *text;
        } asTerminal;
        struct {
            cchar *functionName;
            AstExpr *firstParam; /* TODO: this? */
        } asFuncCall;
    };
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

    union {
        AstRoot *asRoot;
        AstExpr *asReturn;
        struct {
            cchar *symbol;
            AstExpr *expr;
        } asAssign;
        void *asRead;  /* TODO */
        void *asWrite; /* TODO */
    };
};

AstStatement *makeStatement(StatementType type);

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
    AstExpr *comparison;
    AstStatement *firstStatement;
};

struct Program {
    AstRoot root;
    cchar *firstError; /* TODO: is this worth it? */

    u32 blockCount;
    u32 tempCount;

    size_t listCount;
    size_t stackHead;

    /* Master list of all tables */
    SymbolTable tableList[MAX_TABLES];

    /* stack representing nesting, tables duplicated in tableList */
    SymbolTable *tableStack[MAX_TABLES];
};

#endif

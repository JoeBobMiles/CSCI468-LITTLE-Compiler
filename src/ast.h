#ifndef ast_h
#define ast_h

#include "main.h"

struct Program;
struct AstRoot;
struct AstStatement;

#include "symbol-table.h"

#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>

#include "simplify-antlr-types.h"

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
        AstExpr *asRead;
        AstExpr *asWrite;
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
    AstStatement *firstStatement;
};

struct AstWhileRoot {
    AstRoot header;
    AstExpr *comparison;
};

struct AstIfRoot {
    AstRoot header;
    AstExpr *comparison;
    AstRoot *elsePart;
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

SymbolTable *openNewScope(Program *program, cchar *scopeName);
void openScope(Program *program, SymbolTable *scope);
SymbolTable *getScope(Program *program);
SymbolEntry *findDecl(Program *program, cchar *id);
void closeScope(Program *program);

void addDeclarations(Program *program, DeclContext *decl);
AstStatement *addStatements(Program *program, StmtListContext *stmtList);

AstExpr *astFromId(Program *program, IdContext *id);
AstExpr *astFromIdList(Program *program, IdListContext *list);
AstExpr *astFromExprPrefix(Program *program, ExprPrefixContext *prefix);
AstExpr *astFromPostfixExpr(Program *program, PostfixExprContext *postfix);
AstExpr *astFromFactorPrefix(Program *program, FactorPrefixContext *prefix);
AstExpr *astFromFactor(Program *program, FactorContext *factor);
AstExpr *astFromExpr(Program *program, ExprContext *expr);
AstExpr *astFromCond(Program *program, CondContext *cond);

void freeExpr(AstExpr *expr);
void freeRoot(AstRoot *root);
void freeProgram(Program *program);

AstRoot *makeFuncRoot(Program *program, FuncDeclContext *ctx, cchar *id);
Program *makeProgram(FileContext *ctx);

static inline
cchar *getNewBlockName(Program *program) {
    static char buffer[256];
    snprintf(buffer, sizeof buffer, "BLOCK %d", ++program->blockCount);
    return buffer;
}

static inline
u32 getNewTempNumber(Program *program) {
    return program->tempCount++;
}

static inline
cchar *typeString(char type) {
    cchar *result = 0;

    switch (type) {
    case 'i': result = "INT"; break;
    case 'f': result = "FLOAT"; break;
    case 'v': result = "VOID"; break;
    case 's': result = "STRING"; break;
    InvalidDefaultCase;
    }

    return result;
}

#endif

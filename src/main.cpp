#include "main.h"

#define STEP3_OUTPUT 1

#include "strings.h"
#include "symbol-table.h"
#include "ast.h"

#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>
#include <TINYVisitor.h>
#include <TINYListener.h>
#include <TINYBaseVisitor.h>
#include <TINYBaseListener.h>

#include "simplify-antlr-types.h"

#include <cstdio>

#include <fstream>
#include <string>

using namespace antlr4;
using namespace std;

static inline
cchar *typeString(char type) {
    cchar *result = 0;

    switch (type) {
    case 'i': result = "INT"; break;
    case 'f': result = "FLOAT"; break;
    case 'v': result = "VOID"; break;
    case 's': result = "STRING"; break;
    default: InvalidCodePath;
    }

    return result;
}

static inline
SymbolTable *openNewScope(Program *program, cchar *scopeName) {
    assert(program->listCount < MAX_TABLES);

    SymbolTable *table = program->tableList + program->listCount++;

    initSymbolTable(table, saveString(scopeName), 16);

    program->tableStack[program->stackHead++] = table;
    return table;
}

static inline
void openNewScope(Program *program, string scopeName) {
    openNewScope(program, scopeName.c_str());
}

static inline
SymbolTable *getScope(Program *program) {
    assert(program->stackHead > 0);
    return program->tableStack[program->stackHead - 1];
}

static inline
SymbolEntry *findDecl(Program *program, cchar *id) {
    SymbolEntry *result = 0;

    u32 stackHead = program->stackHead;
    while (stackHead && !result) {
        SymbolTable *scope = program->tableStack[--stackHead];
        result = getSymbol(scope, id);
    }

    return result;
}

static inline
cchar *makeBlockName(Program *program) {
    static char buffer[256];
    snprintf(buffer, sizeof buffer, "BLOCK %d", ++program->blockCount);
    return buffer;
}

static inline
void closeScope(Program *program) {
    assert(program->stackHead > 0);
    --program->stackHead;
}

void addDeclarations(Program *program, DeclContext *decl) {
    SymbolTable *scope = getScope(program);

    while (decl && !decl->empty()) {
        if (decl->stringDecl()) {
            StringDeclContext *stringDecl = decl->stringDecl();
            assert(stringDecl);

            char *id = saveString(stringDecl->id()->getText().c_str());
            char *value = saveString(stringDecl->str()->getText().c_str());

            if (!addVar(scope, id, 's', value) && !program->firstError) {
                program->firstError = id;
            }
        }
        else {
            VarDeclContext *varDecl = decl->varDecl();
            assert(varDecl);

            char type = tolower(varDecl->varType()->getText()[0]);

            IdListContext *idList = varDecl->idList();
            char *id = saveString(idList->id()->getText().c_str());

            if (!addVar(scope, id, type, 0) && !program->firstError) {
                program->firstError = id;
            }

            IdTailContext *idTail = idList->idTail();
            while (idTail && idTail->id()) {
                id = saveString(idTail->id()->getText().c_str());

                if (!addVar(scope, id, type, 0) && !program->firstError) {
                    program->firstError = id;
                }

                idTail = idTail->idTail();
            }
        }

        decl = decl->decl();
        assert(decl);
    }
}

AstStatement *addStatements(Program *program, StmtListContext *stmtList) {
    //SymbolTable *scope = getScope(program);
    AstStatement *result = 0;
    AstStatement **resultPtr = &result;

    while (!stmtList->empty()) {
        StmtContext *stmt = stmtList->stmt();

        if (stmt->baseStmt()) {
            BaseStmtContext *base = stmt->baseStmt();
            *resultPtr = makeStatement(STATEMENT_Null);

            if (base->assignStmt()) {
                AssignStmtContext *assignStmt = base->assignStmt();
                AssignExprContext *assignExpr = assignStmt->assignExpr();
                IdContext *id = assignExpr->id();

                SymbolEntry *symbol = findDecl(program, id->getText().c_str());
                assert(symbol); /* TODO: don't assert, check! */

                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Assign,
                    .asAssign = {
                        .symbol = symbol->id,
                        .expr = 0, /* TODO */
                    },
                };
            }
            else if (base->readStmt()) {
                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Read,
                    .asRead = 0, /* TODO */
                };
            }
            else if (base->writeStmt()) {
                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Write,
                    .asWrite = 0, /* TODO */
                };
            }
            else if (base->returnStmt()) {
                **resultPtr = (AstStatement){
                    .nextStatement = 0,
                    .type = STATEMENT_Return,
                    .asReturn = 0, /* TODO */
                };
            }
            else InvalidCodePath;

            resultPtr = &(**resultPtr).nextStatement;
        }
        else if (stmt->ifStmt()) {
            IfStmtContext *ifStmt = stmt->ifStmt();

            cchar *blockName = makeBlockName(program);
            SymbolTable *ifScope = openNewScope(program, blockName);
            *resultPtr = makeStatement(STATEMENT_Root);

            addDeclarations(program, ifStmt->decl());

            StmtListContext *stmtList = ifStmt->stmtList();
            (**resultPtr).asRoot = (AstRoot *)malloc(sizeof (AstRoot));
            *(**resultPtr).asRoot = (AstRoot){
                .type = ROOT_If,
                .symbols = ifScope,
                .comparison = 0, /* TODO: this */
                .firstStatement = addStatements(program, stmtList),
            };

            resultPtr = &(**resultPtr).nextStatement;
            closeScope(program);

            ElsePartContext *elseStmt = ifStmt->elsePart();
            if (!elseStmt->empty()) {
                cchar *blockName = makeBlockName(program);
                SymbolTable *elseScope = openNewScope(program, blockName);
                *resultPtr = makeStatement(STATEMENT_Root);

                addDeclarations(program, elseStmt->decl());

                StmtListContext *stmtList = elseStmt->stmtList();
                (**resultPtr).asRoot = (AstRoot *)malloc(sizeof (AstRoot));
                *(**resultPtr).asRoot = (AstRoot){
                    .type = ROOT_Else,
                    .symbols = elseScope,
                    .comparison = 0, /* TODO: this */
                    .firstStatement = addStatements(program, stmtList),
                };

                resultPtr = &(**resultPtr).nextStatement;
                closeScope(program);
            }
        }
        else if (stmt->whileStmt()) {
            WhileStmtContext *whileStmt = stmt->whileStmt();

            cchar *blockName = makeBlockName(program);
            SymbolTable *whileScope = openNewScope(program, blockName);
            *resultPtr = makeStatement(STATEMENT_Root);

            addDeclarations(program, whileStmt->decl());

            StmtListContext *stmtList = whileStmt->stmtList();
            (**resultPtr).asRoot = (AstRoot *)malloc(sizeof (AstRoot));
            *(**resultPtr).asRoot = (AstRoot){
                .type = ROOT_While,
                .symbols = whileScope,
                .comparison = 0, /* TODO: this */
                .firstStatement = addStatements(program, stmtList),
            };

            resultPtr = &(**resultPtr).nextStatement;
            closeScope(program);
        }
        else InvalidCodePath;

        stmtList = stmtList->stmtList();
    }

    return result;
}

void freeRoot(AstRoot *root) {
    assert(root);

    AstStatement *statement = root->firstStatement;
    while (statement) {
        AstStatement *nextStatement = statement->nextStatement;

        /* TODO: free statement, if it's a root, recurs into it */
        if (statement->type == STATEMENT_Root) {
            freeRoot(statement->asRoot);
        }

        free((void *) statement);
        statement = nextStatement;
    }

    free((void *)root);
}

AstRoot *makeFuncRoot(Program *program, FuncDeclContext *ctx, cchar *id) {
    assert(program);
    assert(ctx);
    assert(id);

    SymbolTable *scope = openNewScope(program, id);

    AstRoot *root = (AstRoot *)malloc(sizeof *root);
    *root = (AstRoot){
        .type = ROOT_Function,
        .symbols = scope,
        .comparison = 0, /* TODO: this */
        .firstStatement = 0,
    };

    /*
     * Parameters
     */

    ParamDeclListContext *paramList = ctx->paramDeclList();
    if (!paramList->empty()) {
        ParamDeclContext *param = paramList->paramDecl();

        char *id = saveString(param->id()->getText().c_str());
        char type = tolower(param->varType()->getText()[0]);

        if (!addVar(scope, id, type, 0) && !program->firstError) {
            program->firstError = id;
        }

        ParamDeclTailContext *paramTail = paramList->paramDeclTail();
        while (!paramTail->empty()) {
            param = paramTail->paramDecl();

            id = saveString(param->id()->getText().c_str());
            type = tolower(param->varType()->getText()[0]);

            if (!addVar(scope, id, type, 0) && !program->firstError) {
                program->firstError = id;
            }

            paramTail = paramTail->paramDeclTail();
            assert(paramTail);
        }
    }

    /*
     * Declarations
     */

    FuncBodyContext *funcBody = ctx->funcBody();
    addDeclarations(program, funcBody->decl());

    /*
     * Walk the Statements
     */

    root->firstStatement = addStatements(program, funcBody->stmtList());

    closeScope(program);

    return root;
}

void freeProgram(Program *program) {
    assert(program);

    for (u32 listIndex = 0; listIndex < program->listCount; ++listIndex) {
        SymbolTable *table = program->tableList + listIndex;

        /* get rid of any roots */
        for (u32 i = 0; i < table->count; ++i) {
            u32 index = table->order[i];
            SymbolEntry entry = table->data[index];

            if (entry.root) {
                freeRoot(entry.root);
            }
        }

        deinitSymbolTable(table);
    }

    free((void *)program);
}

Program *makeProgram(FileContext *ctx) {
    Program *program = (Program *)malloc(sizeof *program);
    *program = (Program){
        .root = (AstRoot){
            .type = ROOT_Global,
            .symbols = 0,
            .comparison = 0,
            .firstStatement = 0,
        },
        .firstError = 0,
        .blockCount = 0,
        .listCount = 0,
        .stackHead = 0,
        .tableList = {},
        .tableStack = {},
    };

    PgmBodyContext *pgmBody = ctx->program()->pgmBody();
    SymbolTable *scope = openNewScope(program, "GLOBAL");
    program->root.symbols = scope;

    /* TODO: check for errors on our way down */

    /*
     * Declarations
     */

    addDeclarations(program, pgmBody->decl());

    /*
     * Functions
     */

    FuncDeclarationsContext *funcs = pgmBody->funcDeclarations();
    while (!funcs->empty()) {
        FuncDeclContext *funcDecl = funcs->funcDecl();
        assert(funcDecl);

        char returnType = tolower(funcDecl->anyType()->getText()[0]);
        cchar *id = saveString(funcDecl->id()->getText().c_str());
        char paramTypes[32] = {};
        /* NOTE: 31 parameters should be enough for anyone */

        /*
         * build up the parameter type string
         */

        char *cur = paramTypes;
        ParamDeclListContext *paramList = funcDecl->paramDeclList();
        if (!paramList->empty()) {
            ParamDeclContext *param = paramList->paramDecl();

            *cur++ = tolower(param->varType()->getText()[0]);
            assert(cur < paramTypes + sizeof paramTypes - 1);

            ParamDeclTailContext *paramTail = paramList->paramDeclTail();
            while (!paramTail->empty()) {
                param = paramTail->paramDecl();

                *cur++ = tolower(param->varType()->getText()[0]);
                assert(cur < paramTypes + sizeof paramTypes - 1);

                paramTail = paramTail->paramDeclTail();
                assert(paramTail);
            }
        }

        AstRoot *funcRoot = makeFuncRoot(program, funcDecl, id);
        addFunc(scope, id, returnType, saveString(paramTypes), funcRoot);

        funcs = funcs->funcDeclarations();
        assert(funcs);
    }

    closeScope(program);

    return program;
}

void printExpr(AstExpr *expr) {
    printf("-- TODO: expr --");
};

void printRoot(AstRoot *);
void printStatement(AstStatement *statement, cchar *indent) {
    switch (statement->type) {
    case STATEMENT_Root:
        printRoot(statement->asRoot);
        break;

    case STATEMENT_Assign:
        printf("%s%s := ", indent, statement->asAssign.symbol);
        printExpr(statement->asAssign.expr);
        printf(";\n");
        break;

    case STATEMENT_Read:
        printf("%s-- TODO: Read\n", indent);
        break;

    case STATEMENT_Write:
        printf("%s-- TODO: Write\n", indent);
        break;

    case STATEMENT_Return:
        printf("%s-- TODO: Return\n", indent);
        break;

    default: InvalidCodePath;
    }
}

void printRoot(AstRoot *root) {
    /* 64 levels of indent should be enough for anyone */
    static char indent[64]; /* TODO: de-static-ify this? */
    static char *indentCur = indent;

    assert(indentCur >= indent);

    *indentCur = 0; /* cap the current indent amount */

    SymbolTable *symbols = root->symbols;

    switch (root->type) {
    case ROOT_Null:
        printf("%s<NULL ROOT>\n", indent);
        break;

    case ROOT_Global:
        printf("%sPROGRAM root\n", indent);
        break;

    case ROOT_Function: {
        bool firstParam = true;
        printf("%sFUNCTION %s(", indent, symbols->name);
        for (u32 i = 0; i < symbols->count; ++i) {
            u32 index = symbols->order[i];
            SymbolEntry entry = symbols->data[index];

            if (entry.symbolType == 'p') {
                if (firstParam) {
                    printf("%s %s", typeString(entry.logicalType), entry.id);
                    firstParam = false;
                }
                else {
                    printf(", %s %s", typeString(entry.logicalType), entry.id);
                }
            }
        }
        printf(")\n");
    }   break;

    case ROOT_If:
        printf("%s<TODO: IF>\n", indent);
        break;

    case ROOT_Else:
        printf("%s<TODO: ELSE>\n", indent);
        break;

    case ROOT_While:
        printf("%s<TODO: WHILE>\n", indent);
        break;
    }

    printf("%sBEGIN\n", indent);
    *indentCur++ = '\t';

    printf("%s-- Variables:\n\n", indent);
    for (u32 i = 0; i < symbols->count; ++i) {
        u32 index = symbols->order[i];
        SymbolEntry entry = symbols->data[index];

        if (entry.symbolType == 'v') {
            cchar *type = typeString(entry.logicalType);

            printf("%s%s %s", indent, type, entry.id);
            if (entry.value) {
                printf(" := %s", entry.value);
            }
            printf(";\n");
        }
    }

    if (root->type == ROOT_Global) {
        /* print out main() first, because I think the pseudo-asm goes
         * top-to-bottom, and we want to hit main() first */
        printf("\n%s-- Main:\n\n", indent);
        SymbolEntry *mainEntry = getSymbol(symbols, "main");
        if (mainEntry && mainEntry->symbolType == 'f') {
            printRoot(mainEntry->root);
        }

        printf("\n%s-- Other Functions:\n\n", indent);
        for (u32 i = 0; i < symbols->count; ++i) {
            u32 index = symbols->order[i];
            SymbolEntry *entry = symbols->data + index;

            /* ...but skip main() */
            if (entry->symbolType == 'f' && !stringsAreEqual(entry->id, "main")) {
                printRoot(entry->root);
            }
        }
    }
    else {
        printf("\n%s-- Statements:\n\n", indent);

        AstStatement *statement = root->firstStatement;
        while (statement) {
            printStatement(statement, indent);

            statement = statement->nextStatement;
        }
    }

    *--indentCur = '\0';
    printf("%sEND\n", indent);
}

int main(int argc, char **argv) {
    ifstream file(argc == 1? "example.tiny": argv[1]);

    if (file.is_open()) {
        ANTLRInputStream input(file);
        TINYLexer lexer(&input);
        CommonTokenStream tokenStream(&lexer);
        TINYParser parser(&tokenStream);

        FileContext *fileCtx = parser.file();
        Program *program = makeProgram(fileCtx);
        printRoot(&program->root);

        file.close();

        /* free all of our stuff */
        freeProgram(program);
    }

    deinitStringTable(globalStringTable);
}

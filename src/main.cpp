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

void addDeclarations(Program *program, TINYParser::DeclContext *decl) {
    SymbolTable *scope = getScope(program);

    while (decl && !decl->empty()) {
        if (decl->stringDecl()) {
            TINYParser::StringDeclContext *stringDecl = decl->stringDecl();
            assert(stringDecl);

            char *id = saveString(stringDecl->id()->getText().c_str());
            char *value = saveString(stringDecl->str()->getText().c_str());

            if (!addVar(scope, id, 's', value) && !program->firstError) {
                program->firstError = id;
            }
        }
        else {
            TINYParser::VarDeclContext *varDecl = decl->varDecl();
            assert(varDecl);

            char type = tolower(varDecl->varType()->getText()[0]);

            TINYParser::IdListContext *idList = varDecl->idList();
            char *id = saveString(idList->id()->getText().c_str());

            if (!addVar(scope, id, type, 0) && !program->firstError) {
                program->firstError = id;
            }

            TINYParser::IdTailContext *idTail = idList->idTail();
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

AstStatement *addStatements(Program *program, TINYParser::StmtListContext *stmtList) {
    while (!stmtList->empty()) {
        TINYParser::StmtContext *stmt = stmtList->stmt();

        if (stmt->baseStmt()) {
            /* TODO: base statements. These go into the tree */
        }
        else if (stmt->ifStmt()) {
            TINYParser::IfStmtContext *ifStmt = stmt->ifStmt();

            openNewScope(program, makeBlockName(program));
            addDeclarations(program, ifStmt->decl());
            addStatements(program, ifStmt->stmtList());
            closeScope(program);

            TINYParser::ElsePartContext *elsePart = ifStmt->elsePart();
            if (!elsePart->empty()) {
                openNewScope(program, makeBlockName(program));
                addDeclarations(program, elsePart->decl());
                addStatements(program, elsePart->stmtList());
                closeScope(program);
            }
        }
        else if (stmt->whileStmt()) {
            TINYParser::WhileStmtContext *whileStmt = stmt->whileStmt();

            openNewScope(program, makeBlockName(program));
            addDeclarations(program, whileStmt->decl());
            addStatements(program, whileStmt->stmtList());
            closeScope(program);
        }
        else { InvalidCodePath; }

        stmtList = stmtList->stmtList();
    }

    return 0; /* TODO: actually return these statements */
}

void freeFuncRoot(AstRoot *root) {
    free((void *)root);
}

AstRoot *makeFuncRoot(Program *program, TINYParser::FuncDeclContext *ctx, cchar *id) {
    SymbolTable *scope = openNewScope(program, id);

    AstRoot *root = (AstRoot *)malloc(sizeof *root);
    *root = (AstRoot){
        .symbols = scope,
        .firstStatement = 0,
    };

    /*
     * Parameters
     */

    TINYParser::ParamDeclListContext *paramList = ctx->paramDeclList();
    if (!paramList->empty()) {
        TINYParser::ParamDeclContext *param = paramList->paramDecl();

        char *id = saveString(param->id()->getText().c_str());
        char type = tolower(param->varType()->getText()[0]);

        if (!addVar(scope, id, type, 0) && !program->firstError) {
            program->firstError = id;
        }

        TINYParser::ParamDeclTailContext *paramTail = paramList->paramDeclTail();
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

    TINYParser::FuncBodyContext *funcBody = ctx->funcBody();
    addDeclarations(program, funcBody->decl());

    /*
     * Walk the Statements
     */

    addStatements(program, funcBody->stmtList());

    closeScope(program);

    return root;
}

void freeProgram(Program *program) {
    for (size_t listIndex = 0; listIndex < program->listCount; ++listIndex) {
        SymbolTable *table = program->tableList + listIndex;

        /* get rid of any roots */
        for (size_t i = 0; i < table->count; ++i) {
            size_t index = table->order[i];
            SymbolEntry entry = table->data[index];
            if (entry.root) {
                freeFuncRoot(entry.root);
            }
        }

        deinitSymbolTable(table);
    }

    free((void *)program);
}

Program *makeProgram(TINYParser::FileContext *ctx) {
    Program *program = (Program *)malloc(sizeof *program);
    *program = (Program){
        .root = (AstRoot){
            .symbols = 0,
            .firstStatement = 0,
        },
        .firstError = 0,
        .blockCount = 0,
        .listCount = 0,
        .stackHead = 0,
        .tableList = {},
        .tableStack = {},
    };

    TINYParser::PgmBodyContext *pgmBody = ctx->program()->pgmBody();
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

    TINYParser::FuncDeclarationsContext *funcs = pgmBody->funcDeclarations();
    while (!funcs->empty()) {
        TINYParser::FuncDeclContext *funcDecl = funcs->funcDecl();
        assert(funcDecl);

        char returnType = tolower(funcDecl->anyType()->getText()[0]);
        cchar *id = saveString(funcDecl->id()->getText().c_str());
        char paramTypes[32] = {};
        /* NOTE: 31 parameters should be enough for anyone */

        /*
         * build up the parameter type string
         */

        char *cur = paramTypes;
        TINYParser::ParamDeclListContext *paramList = funcDecl->paramDeclList();
        if (!paramList->empty()) {
            TINYParser::ParamDeclContext *param = paramList->paramDecl();

            *cur++ = tolower(param->varType()->getText()[0]);
            assert(cur < paramTypes + sizeof paramTypes - 1);

            TINYParser::ParamDeclTailContext *paramTail = paramList->paramDeclTail();
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

int main(int argc, char **argv) {
    ifstream file(argc == 1? "example.tiny": argv[1]);

    if (file.is_open()) {
        ANTLRInputStream input(file);
        TINYLexer lexer(&input);
        CommonTokenStream tokenStream(&lexer);
        TINYParser parser(&tokenStream);

        TINYParser::FileContext *fileCtx = parser.file();
        Program *program = makeProgram(fileCtx);

        file.close();

        /* copy some things out for convenience */
        cchar *firstError = program->firstError;
        size_t listCount = program->listCount;
        SymbolTable *tableList = program->tableList;

        if (!firstError) {
            for (size_t listIndex = 0; listIndex < listCount; ++listIndex) {
                SymbolTable *table = tableList + listIndex;

                printf("Symbol table %s\n", table->name);

                for (size_t i = 0; i < table->count; ++i) {
                    size_t index = table->order[i];
                    SymbolEntry entry = table->data[index];

                    if (entry.id) {
                        switch (entry.symbolType) {
                        case 'v': {
                            cchar *type = typeString(entry.logicalType);

#if STEP3_OUTPUT
                            printf("name %s type %s", entry.id, type);
                            if (entry.value) {
                                printf(" value %s", entry.value);
                            }
                            printf("\n");
#else
                            printf("var %s %s", type, entry.id);
                            if (entry.value) {
                                printf(" := %s", entry.value);
                            }
                            printf(";\n");
#endif
                        } break;

                        case 'f': {
#if !STEP3_OUTPUT
                            cchar *type = typeString(entry.logicalType);
                            assert(entry.value);

                            printf("func %s %s(", type, entry.id);

                            cchar *cur = entry.value;
                            if (*cur) {
                                printf("%s", typeString(*cur++));
                                while (*cur) {
                                    printf(", %s", typeString(*cur++));
                                }
                            }
                            printf(");\n");
#endif
                        } break;

                        default: InvalidCodePath;
                        }
                    }
                }

                printf("\n");
            }
        }
        else {
            printf("DECLARATION ERROR %s\n", firstError);
        }

        /* free all of our stuff */
        freeProgram(program);
    }

    deinitStringTable(globalStringTable);
}

#include "main.h"

#define STEP3_OUTPUT 0

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
void closeScope(Program *program) {
    assert(program->stackHead > 0);
    --program->stackHead;
}

static const string blockString = "BLOCK ";
class OurListener : public TINYBaseListener {
public:
    int blockNumber = 0;
    Program program = {
        .listCount = 0,
        .stackHead = 0,
        .firstError = 0,
        .tableList = {},
        .tableStack = {},
    };

    virtual void enterFuncDecl(TINYParser::FuncDeclContext *ctx) override {
        openNewScope(&program, ctx->id()->getText());
    }
    virtual void exitFuncDecl(TINYParser::FuncDeclContext *ctx) override {
        closeScope(&program);
    }

    virtual void enterIfStmt(TINYParser::IfStmtContext *ctx) override {
        openNewScope(&program, blockString + to_string(++blockNumber));
    }
    virtual void exitIfStmt(TINYParser::IfStmtContext *ctx) override {
        closeScope(&program);
    }

    virtual void enterElsePart(TINYParser::ElsePartContext *ctx) override {
        if (!ctx->empty()) {
            openNewScope(&program, blockString + to_string(++blockNumber));
        }
    }
    virtual void exitElsePart(TINYParser::ElsePartContext *ctx) override {
        if (!ctx->empty()) {
            closeScope(&program);
        }
    }

    virtual void enterWhileStmt(TINYParser::WhileStmtContext *ctx) override {
        openNewScope(&program, blockString + to_string(++blockNumber));
    }
    virtual void exitWhileStmt(TINYParser::WhileStmtContext *ctx) override {
        closeScope(&program);
    }

    virtual void enterParamDecl(TINYParser::ParamDeclContext *ctx) override {
        SymbolTable *table = getScope(&program);

        char *id = saveString(ctx->id()->getText().c_str());
        char type = tolower(ctx->varType()->getText()[0]);

        if (!addVar(table, id, type, 0) && !program.firstError) {
            program.firstError = id;
        }
    }

#if 1
    virtual void enterEveryRule(antlr4::ParserRuleContext *ctx) override {
        if (ctx->exception) {
            /* TODO: indicate error without dieing */
            throw ctx->exception;
        }
        //printf("%s\n", ctx->getText().c_str());
    }
    //virtual void exitEveryRule(antlr4::ParserRuleContext *ctx) override { }
#endif
};

void freeProgram(Program *program) {
    for (size_t listIndex = 0; listIndex < program->listCount; ++listIndex) {
        deinitSymbolTable(program->tableList + listIndex);
    }

    free((void *)program);
}

Program *makeProgram(TINYParser::FileContext *ctx) {
    Program *program = (Program *)malloc(sizeof *program);
    *program = (Program){
        .listCount = 0,
        .stackHead = 0,
        .firstError = 0,
        .tableList = {},
        .tableStack = {},
    };

    /* TODO: check for errors on our way down */

    TINYParser::PgmBodyContext *pgmBody = ctx->program()->pgmBody();
    SymbolTable *scope = openNewScope(program, "GLOBAL");

    /*
     * Take care of variable declarations.
     */

    TINYParser::DeclContext *decl = pgmBody->decl();
    while (!decl->empty()) {
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

    /*
     * Take care of function declarations and recurs into them.
     */

    TINYParser::FuncDeclarationsContext *funcs = pgmBody->funcDeclarations();
    while (!funcs->empty()) {
        TINYParser::FuncDeclContext *funcDecl = funcs->funcDecl();
        assert(funcDecl);

        char returnType = tolower(funcDecl->anyType()->getText()[0]);
        cchar *id = saveString(funcDecl->id()->getText().c_str());
        char paramTypes[32] = {};
        /* NOTE: 31 parameters should be enough for anyone */

        char *end = paramTypes + sizeof paramTypes - 1;
        char *cur = paramTypes;
        /* TODO: actually form paramTypes */
        *cur++ = 'i';
        *cur++ = 'f';
        *cur++ = 's';
        assert(cur < end);

        /* TODO: add root to entry */
        addFunc(scope, id, returnType, saveString(paramTypes), 0);

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

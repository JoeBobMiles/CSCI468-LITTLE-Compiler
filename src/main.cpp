#include "main.h"

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
void openNewScope(Program *program, cchar *scopeName) {
    assert(program->listCount < MAX_TABLES);

    SymbolTable *table = program->tableList + program->listCount++;

    initSymbolTable(table, saveString(scopeName), 16);

    program->tableStack[program->stackHead++] = table;
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

    virtual void enterProgram(TINYParser::ProgramContext *ctx) override {
        openNewScope(&program, "GLOBAL");
    }
    virtual void exitProgram(TINYParser::ProgramContext *ctx) override {
        closeScope(&program);
    }

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

    virtual void enterVarDecl(TINYParser::VarDeclContext *ctx) override {
        SymbolTable *table = getScope(&program);
        char *type = saveString(ctx->varType()->getText().c_str());

        TINYParser::IdListContext *idList = ctx->idList();
        char *id = saveString(idList->id()->getText().c_str());

        if (!addSymbol(table, id, type) && !program.firstError) {
            program.firstError = id;
        }

        TINYParser::IdTailContext *idTail = idList->idTail();
        while (idTail && idTail->id()) {
            id = saveString(idTail->id()->getText().c_str());

            if (!addSymbol(table, id, saveString(type)) && !program.firstError) {
                program.firstError = id;
            }

            idTail = idTail->idTail();
        }
    }

    virtual void enterStringDecl(TINYParser::StringDeclContext *ctx) override {
        SymbolTable *table = getScope(&program);
        char *id = saveString(ctx->id()->getText().c_str());
        char *type = saveString(ctx->STRING()->getText().c_str());
        char *value = saveString(ctx->str()->getText().c_str());

        if (!addSymbol(table, id, type, value) && !program.firstError) {
            program.firstError = id;
        }
    }

    virtual void enterParamDecl(TINYParser::ParamDeclContext *ctx) override {
        SymbolTable *table = getScope(&program);

        char *id = saveString(ctx->id()->getText().c_str());
        char *type = saveString(ctx->varType()->getText().c_str());

        if (!addSymbol(table, id, type) && !program.firstError) {
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

int main(int argc, char **argv) {
    ifstream file(argc == 1? "example.tiny": argv[1]);

    if (file.is_open()) {
        ANTLRInputStream input(file);
        TINYLexer lexer(&input);
        CommonTokenStream tokenStream(&lexer);
        TINYParser parser(&tokenStream);

        OurListener listener; /* TODO: ditch the listener */
        tree::ParseTree *tree = parser.file();
        tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

        file.close();

        /* copy some things out for convenience */
        cchar *firstError = listener.program.firstError;
        size_t listCount = listener.program.listCount;
        SymbolTable *tableList = listener.program.tableList;

        if (!firstError) {
            for (size_t listIndex = 0; listIndex < listCount; ++listIndex) {
                SymbolTable *table = tableList + listIndex;

                printf("Symbol table %s\n", table->name);

                for (size_t i = 0; i < table->count; ++i) {
                    size_t index = table->order[i];
                    SymbolEntry entry = table->data[index];

                    if (entry.id) {
                        printf("name %s type %s", entry.id, entry.type);
                        if (entry.value) {
                            printf(" value %s", entry.value);
                        }
                        printf("\n");
                    }
                }

                printf("\n");
            }
        }
        else {
            printf("DECLARATION ERROR %s\n", firstError);
        }

        /* free all of our stuff */
        for (size_t listIndex = 0; listIndex < listCount; ++listIndex) {
            deinitSymbolTable(tableList + listIndex);
        }
    }

    deinitStringTable(globalStringTable);
}

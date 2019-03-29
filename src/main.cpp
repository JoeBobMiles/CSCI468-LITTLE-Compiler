#include "main.h"

#include "symbol-table.h"
#include "strings.h"

#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>
#include <TINYVisitor.h>
#include <TINYListener.h>
#include <TINYBaseVisitor.h>
#include <TINYBaseListener.h>

#include <cstdio>
//#include <cstring>

#include <fstream>
#include <string>

using namespace antlr4;
using namespace std;

#define MAX_TABLES 32 /* 32 tables should be enough for anyone */

/* Master list of all tables */
size_t listCount = 0;
SymbolTable tableList[MAX_TABLES];

/* stack representing nesting, tables duplicated in tableList */
size_t stackHead = 0;
SymbolTable *tableStack[MAX_TABLES];

string firstError;

static inline
void openNewScope(cchar *scopeName) {
    assert(listCount < MAX_TABLES);

    SymbolTable *table = tableList + listCount++;

    initSymbolTable(table, scopeName, 16);

    tableStack[stackHead++] = table;
}

static inline
void openNewScope(string scopeName) {
    openNewScope(saveString(scopeName.c_str()));
}

static inline
SymbolTable *getScope() {
    assert(stackHead > 0);
    return tableStack[stackHead - 1];
}

static inline
void closeScope() {
    assert(stackHead > 0);
    --stackHead;
}

static const string blockString = "BLOCK ";
class OurListener : public TINYBaseListener {
public:
    int blockNumber = 0;

    virtual void enterProgram(TINYParser::ProgramContext *ctx) override {
        openNewScope(saveString("GLOBAL"));
    }
    virtual void exitProgram(TINYParser::ProgramContext *ctx) override {
        closeScope();
    }

    virtual void enterFuncDecl(TINYParser::FuncDeclContext *ctx) override {
        openNewScope(ctx->id()->getText());
    }
    virtual void exitFuncDecl(TINYParser::FuncDeclContext *ctx) override {
        closeScope();
    }

    virtual void enterIfStmt(TINYParser::IfStmtContext *ctx) override {
        openNewScope(blockString + to_string(++blockNumber));
    }
    virtual void exitIfStmt(TINYParser::IfStmtContext *ctx) override {
        closeScope();
    }

    virtual void enterElsePart(TINYParser::ElsePartContext *ctx) override {
        if (!ctx->empty()) {
            openNewScope(blockString + to_string(++blockNumber));
        }
    }
    virtual void exitElsePart(TINYParser::ElsePartContext *ctx) override {
        if (!ctx->empty()) {
            closeScope();
        }
    }

    virtual void enterWhileStmt(TINYParser::WhileStmtContext *ctx) override {
        openNewScope(blockString + to_string(++blockNumber));
    }
    virtual void exitWhileStmt(TINYParser::WhileStmtContext *ctx) override {
        closeScope();
    }

    virtual void enterVarDecl(TINYParser::VarDeclContext *ctx) override {
        SymbolTable *table = getScope();
        char *type = saveString(ctx->varType()->getText().c_str());

        TINYParser::IdListContext *idList = ctx->idList();
        char *id = saveString(idList->id()->getText().c_str());

        if (!addSymbol(table, id, type) && firstError.empty()) {
            firstError = string(id);
        }

        TINYParser::IdTailContext *idTail = idList->idTail();
        while (idTail && idTail->id()) {
            id = saveString(idTail->id()->getText().c_str());

            if (!addSymbol(table, id, saveString(type)) && firstError.empty()) {
                firstError = string(id);
            }

            idTail = idTail->idTail();
        }
    }

    virtual void enterStringDecl(TINYParser::StringDeclContext *ctx) override {
        SymbolTable *table = getScope();
        char *id = saveString(ctx->id()->getText().c_str());
        char *type = saveString(ctx->STRING()->getText().c_str());
        char *value = saveString(ctx->str()->getText().c_str());

        if (!addSymbol(table, id, type, value) && firstError.empty()) {
            firstError = string(id);
        }
    }

    virtual void enterParamDecl(TINYParser::ParamDeclContext *ctx) override {
        SymbolTable *table = getScope();

        char *id = saveString(ctx->id()->getText().c_str());
        char *type = saveString(ctx->varType()->getText().c_str());

        if (!addSymbol(table, id, type) && firstError.empty()) {
            firstError = string(id);
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

        /* TODO: attach listener prior to parsing? */
        OurListener listener;
        tree::ParseTree *tree = parser.file();
        tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

        file.close();
    }

    if (firstError.empty()) {
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

            /* free all of our stuff */
            deinitSymbolTable(table);
        }
    }
    else {
        printf("DECLARATION ERROR %s\n", firstError.c_str());
    }

    deinitStringTable(globalStringTable);
}

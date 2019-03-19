#include "main.h"

#include "symbol-table.h"

#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>
#include <TINYVisitor.h>
#include <TINYListener.h>
#include <TINYBaseVisitor.h>
#include <TINYBaseListener.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace antlr4;
using namespace std;

#define MAX_TABLES 32 /* 32 tables should be enough for anyone */

/* Master list of all tables */
size_t listCount = 0;
SymbolTable tableList[MAX_TABLES];

/* stack representing nesting, tables duplicated in tableList */
size_t stackHead = 0;
SymbolTable *tableStack[MAX_TABLES];

static inline
void openNewScope(string scopeName) {
    assert(listCount < MAX_TABLES);

    SymbolTable *table = tableList + listCount++;

    initSymbolTable(table, scopeName, 16);

    tableStack[stackHead++] = table;
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
        openNewScope("GLOBAL");
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
        openNewScope(blockString + to_string(++blockNumber));
    }
    virtual void exitElsePart(TINYParser::ElsePartContext *ctx) override {
        closeScope();
    }

    virtual void enterWhileStmt(TINYParser::WhileStmtContext *ctx) override {
        openNewScope(blockString + to_string(++blockNumber));
    }
    virtual void exitWhileStmt(TINYParser::WhileStmtContext *ctx) override {
        closeScope();
    }

    virtual void enterVarDecl(TINYParser::VarDeclContext *ctx) override {
        SymbolTable *table = getScope();
        string type = ctx->varType()->getText();

        TINYParser::IdListContext *idList = ctx->idList();
        string idText = idList->id()->getText();
        if (!addSymbol(table, idText, type)) {
            cout << "DECLARATION ERROR " << idText;
        }

        TINYParser::IdTailContext *idTail = idList->idTail();
        while (idTail && idTail->id()) {
            idText = idTail->id()->getText();

            if (!addSymbol(table, idText, type)) {
                cout << "DECLARATION ERROR " << idText;
            }

            idTail = idTail->idTail();
        }
    }

    virtual void enterStringDecl(TINYParser::StringDeclContext *ctx) override {
        SymbolTable *table = getScope();
        string id = ctx->id()->getText();
        string type = ctx->STRING()->getText();
        string value = ctx->str()->getText();

        if (!addSymbol(table, id, type, value)) {
            cout << "DECLARATION ERROR " << id;
        }
    }

    virtual void enterParamDecl(TINYParser::ParamDeclContext *ctx) override {
        SymbolTable *table = getScope();

        string id = ctx->id()->getText();
        string type = ctx->varType()->getText();

        if (!addSymbol(table, id, type)) {
            cout << "DECLARATION ERROR " << id;
        }
    }

#if 1
    virtual void enterEveryRule(antlr4::ParserRuleContext *ctx) override {
        if (ctx->exception) {
            /* TODO: indicate error */
        }
        //cout << ctx->getText() << '\n';
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

    for (size_t listIndex = 0; listIndex < listCount; ++listIndex) {
        SymbolTable *table = tableList + listIndex;

        /* c++ style output is gross. Unfortunately, our hand has been forced,
         * because it's infeasable to use c-style strings. It turns out
         * string::c_str() returns a pointer to temporary memory, meaning that
         * having more than one in a function call is a problem. We could deal
         * with it, but I think it will cause too many inconveniences in the
         * future */
        cout << "Symbol table " << table->name << '\n';

        for (size_t index = 0; index < table->size; ++index) {
            SymbolEntry entry = table->data[index];

            if (!entry.id.empty()) {
                cout << "name " << entry.id;
                cout << " type " << entry.type;
                if (!entry.value.empty()) {
                    cout << " value " << entry.value;
                }
                cout << '\n';
            }
        }

        cout << '\n';
    }
}

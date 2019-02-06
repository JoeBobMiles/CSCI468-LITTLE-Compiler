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

using namespace antlr4;
using namespace std;
using namespace std::string_literals;

class OurListener : public TINYBaseListener {
public:
  virtual void enterExpr(TINYParser::ExprContext *ctx) override {
      printf("expr: %s\n", ctx->getText().c_str());
  }

  virtual void enterEveryRule(antlr4::ParserRuleContext *ctx) override {
      printf("%s\n", ctx->getText().c_str());
  }
  //virtual void exitEveryRule(antlr4::ParserRuleContext *ctx) override { }

  virtual void visitTerminal(antlr4::tree::TerminalNode *node) override {
      printf("> %s\n", node->toString().c_str());
  }
  //virtual void visitErrorNode(antlr4::tree::ErrorNode *node) override { }
};

int main(int argc, char **argv) {
    ifstream file(argc == 1? "example.tiny": argv[1]);

    if (file.is_open()) {
        ANTLRInputStream input(file);
        TINYLexer lexer(&input);
        CommonTokenStream tokenStream(&lexer);
        TINYParser parser(&tokenStream);

        OurListener listener;
        tree::ParseTree *tree = parser.expr();
        tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

        file.close();
    }
}

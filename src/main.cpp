#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>
#include <TINYVisitor.h>
#include <TINYListener.h>
#include <TINYBaseVisitor.h>
#include <TINYBaseListener.h>

#include <fstream>
#include <string>

using namespace antlr4;
using namespace std;

#if 0
class OurListener : public TINYBaseListener {
public:
  virtual void enterFile(TINYParser::FileContext *ctx) override {
      printf("file: %s\n\n", ctx->getText().c_str());
  }


  virtual void enterEveryRule(antlr4::ParserRuleContext *ctx) override {
      printf("%s\n", ctx->getText().c_str());
  }
  //virtual void exitEveryRule(antlr4::ParserRuleContext *ctx) override { }

  virtual void visitTerminal(antlr4::tree::TerminalNode *node) override {
      printf("   %s\n", node->toString().c_str());
  }
  //virtual void visitErrorNode(antlr4::tree::ErrorNode *node) override { }
};
#endif

char *typeToString(size_t typeId) {
    switch (typeId) {
    case TINYLexer::PROGRAM:       return "KEYWORD";
    case TINYLexer::BEGIN:         return "KEYWORD";
    case TINYLexer::END:           return "KEYWORD";
    case TINYLexer::FUNCTION:      return "KEYWORD";
    case TINYLexer::READ:          return "KEYWORD";
    case TINYLexer::WRITE:         return "KEYWORD";
    case TINYLexer::IF:            return "KEYWORD";
    case TINYLexer::ELSE:          return "KEYWORD";
    case TINYLexer::ENDIF:         return "KEYWORD";
    case TINYLexer::WHILE:         return "KEYWORD";
    case TINYLexer::ENDWHILE:      return "KEYWORD";
    case TINYLexer::CONTINUE:      return "KEYWORD";
    case TINYLexer::BREAK:         return "KEYWORD";
    case TINYLexer::RETURN:        return "KEYWORD";
    case TINYLexer::INT:           return "KEYWORD";
    case TINYLexer::VOID:          return "KEYWORD";
    case TINYLexer::STRING:        return "KEYWORD";
    case TINYLexer::FLOAT:         return "KEYWORD";
    case TINYLexer::COLONEQ:       return "OPERATOR";
    case TINYLexer::PLUS:          return "OPERATOR";
    case TINYLexer::MINUS:         return "OPERATOR";
    case TINYLexer::STAR:          return "OPERATOR";
    case TINYLexer::SLASH:         return "OPERATOR";
    case TINYLexer::EQUAL:         return "OPERATOR";
    case TINYLexer::NOTEQ:         return "OPERATOR";
    case TINYLexer::LT:            return "OPERATOR";
    case TINYLexer::GT:            return "OPERATOR";
    case TINYLexer::OPENPAREN:     return "OPERATOR";
    case TINYLexer::CLOSEPAREN:    return "OPERATOR";
    case TINYLexer::SEMICOLON:     return "OPERATOR";
    case TINYLexer::COMMA:         return "OPERATOR";
    case TINYLexer::LTEQ:          return "OPERATOR";
    case TINYLexer::GTEQ:          return "OPERATOR";
    case TINYLexer::IDENTIFIER:    return "IDENTIFIER";
    case TINYLexer::INTLITERAL:    return "INTLITERAL";
    case TINYLexer::FLOATLITERAL:  return "FLOATLITERAL";
    case TINYLexer::STRINGLITERAL: return "STRINGLITERAL";
    case TINYLexer::COMMENT:       return "COMMENT";
    case TINYLexer::ALPHANUM:      return "ALPHANUM";
    case TINYLexer::ALPHA:         return "ALPHA";
    case TINYLexer::NUMBER:        return "NUMBER";
    case TINYLexer::WS:            return "WS";
    default:                       return "EOF";
    }
}

int main(int argc, char **argv) {
    ifstream file(argc == 1? "example.tiny": argv[1]);

    if (file.is_open()) {
        ANTLRInputStream input(file);
        TINYLexer lexer(&input);
        CommonTokenStream tokenStream(&lexer);
#if 1 /* step 1 */

        tokenStream.fill();
        for (Token *token : tokenStream.getTokens()) {
            size_t type = token->getType();
            string text = token->getText();

            printf("Token Type: %s\n", typeToString(type));
            printf("Value: %s\n", text.c_str());
        }
#else /* step 2? */
        TINYParser parser(&tokenStream);

        OurListener listener;
        tree::ParseTree *tree = parser.file();
        tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
#endif

        file.close();
    }
}

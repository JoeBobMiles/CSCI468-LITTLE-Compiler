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
    case TINYLexer::PROGRAM:       return "PROGRAM";
    case TINYLexer::BEGIN:         return "BEGIN";
    case TINYLexer::END:           return "END";
    case TINYLexer::FUNCTION:      return "FUNCTION";
    case TINYLexer::READ:          return "READ";
    case TINYLexer::WRITE:         return "WRITE";
    case TINYLexer::IF:            return "IF";
    case TINYLexer::ELSE:          return "ELSE";
    case TINYLexer::ENDIF:         return "ENDIF";
    case TINYLexer::WHILE:         return "WHILE";
    case TINYLexer::ENDWHILE:      return "ENDWHILE";
    case TINYLexer::CONTINUE:      return "CONTINUE";
    case TINYLexer::BREAK:         return "BREAK";
    case TINYLexer::RETURN:        return "RETURN";
    case TINYLexer::INT:           return "INT";
    case TINYLexer::VOID:          return "VOID";
    case TINYLexer::STRING:        return "STRING";
    case TINYLexer::FLOAT:         return "FLOAT";
    case TINYLexer::COLONEQ:       return "COLONEQ";
    case TINYLexer::PLUS:          return "PLUS";
    case TINYLexer::MINUS:         return "MINUS";
    case TINYLexer::STAR:          return "STAR";
    case TINYLexer::SLASH:         return "SLASH";
    case TINYLexer::EQUAL:         return "EQUAL";
    case TINYLexer::NOTEQ:         return "NOTEQ";
    case TINYLexer::LT:            return "LT";
    case TINYLexer::GT:            return "GT";
    case TINYLexer::OPENPAREN:     return "OPENPAREN";
    case TINYLexer::CLOSEPAREN:    return "CLOSEPAREN";
    case TINYLexer::SEMICOLON:     return "SEMICOLON";
    case TINYLexer::COMMA:         return "COMMA";
    case TINYLexer::LTEQ:          return "LTEQ";
    case TINYLexer::GTEQ:          return "GTEQ";
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

            printf("Type   %s\n", typeToString(type));
            printf("Text   %s\n", text.c_str());
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

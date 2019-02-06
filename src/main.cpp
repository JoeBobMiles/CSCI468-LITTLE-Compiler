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

void bufferString(char *buffer, size_t size, char *string) {
    char *end = buffer + size - 1;
    char *scur = string;
    char *dcur = buffer;

    do {
        *dcur = *scur;
    }
    while (*scur++ && dcur++ < end);

    *end = 0; /* force null terminator */
}

char *typeToString(size_t typeId) {
    int found = 0;
    static char buffer[0x100];
    size_t foundTypeId;

    FILE *file = fopen("antlr/TINY.tokens", "r");

    for (;;) {
        int ret = fscanf(file, "%[^=]=%lu\n", buffer, &foundTypeId);
        found = (typeId == foundTypeId);

        if (found || ret == -1) break;
    }

    if (!found) {
        bufferString(buffer, sizeof buffer, "UNKNOWN");
    }

    fclose(file);

    return buffer;
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

            if (text == "<EOF>") break;

            printf("Type   %lu, %s\n", type, typeToString(type));
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

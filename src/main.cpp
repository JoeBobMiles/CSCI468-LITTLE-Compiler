#include "main.h"

#define STEP3_OUTPUT 1

#include "strings.h"
#include "symbol-table.h"
#include "ast.h"
#include "printTINY.h"

#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>

#include "simplify-antlr-types.h"

#include <cstdio>

#include <fstream>
#include <string>

using namespace std;
using namespace antlr4;

int main(int argc, char **argv) {
    istream *source = &cin;
    ifstream file;

    if (argc > 1) {
        file.open(argv[1]);
        if (file.is_open()) {
            source = &file;
        }
        else {
            source = 0;
        }
    }

    if (source) {
        ANTLRInputStream input(*source);
        TINYLexer lexer(&input);
        CommonTokenStream tokenStream(&lexer);
        TINYParser parser(&tokenStream);

        FileContext *fileCtx = parser.file();
        Program *program = makeProgram(fileCtx);
        printProgramAsTINY(program);

        if (file.is_open()) {
            file.close();
        }

        /* free all of our stuff */
        freeProgram(program);
    }

    deinitStringTable(globalStringTable);

    return 0;
}

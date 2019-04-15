#include "main.h"

#include "strings.h"
#include "symbol-table.h"
#include "ast.h"
#include "printTINY.h"
#include "printAsm.h"

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
#if PRINT_ASM
        /* TODO: optimizeProgramForAsm(program);
         *
         * This will do things like turn `a := a + 1` into `++a`, which is not
         * supported by the TINY code, but is supported by the asm. Likewise
         * for `a := a - 1` into `--a`. This is the easiest optimization to
         * find, I think, asside from the very basic stuff that we're already
         * doing at print time (like use id instead of reg when we can).
         *
         * Otherwise, this call can also do constant folding, though perhaps
         * only on integers, as we have "infinite" precision for those, but not
         * for floats.
         */
        printProgramAsAsm(program);
#else
        printProgramAsTINY(program);
#endif

        if (file.is_open()) {
            file.close();
        }

        /* free all of our stuff */
        freeProgram(program);
    }

    deinitStringTable(globalStringTable);

    return 0;
}

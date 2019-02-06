#include <antlr4-runtime.h>
#include <TINYLexer.h>
#include <TINYParser.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace antlr4;
using namespace std;
using namespace std::string_literals;

int main(int argc, char **argv) {
    string line;

    /* NOTE: the 's' at the end of the string makes it an `std::string` because
     * of `std::string_literals`. This causes `==` to compare strings rather
     * than compare pointers. */
    if (argc > 1 && "-"s == argv[1]) {
        1;
    }
    ifstream file(argc == 1? "example.tiny": argv[1]);

    /* TODO: hide this stuff out behind a more C-like interface */

    if (file.is_open()) {
        ANTLRInputStream input(file);
        TINYLexer lexer(&input);
        CommonTokenStream tokenStream(&lexer);
        // TINYParser parser(&tokenStream);

        tokenStream.fill();
        for (Token *token : tokenStream.getTokens()) {
            cout << token->toString() << endl;
        }

        file.close();
    }

    /* God to I hate C++ */
}

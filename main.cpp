#include <iostream>

#include "ast.h"
#include "./utils/io.cpp"
#include "parser.cpp"


int main(int argc, char** argv) {
    yydebug = 1;

    if (argc > 1) {
        std::istringstream iStringStream = StreamLinesFromFile(argv[1]);
        lexer = new yyFlexLexer(iStringStream, std::cout);
        yyparse();
        CreateDotFile(Root);
    }
    else {
        std::cout << "not found file" << std::endl;
    }
}
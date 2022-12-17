#include "ast.h"
#include "semantic/semantic.h"

#include "./utils/io.cpp"
#include "parser.cpp"

#include <iostream>
#include <filesystem>

int main(int argc, char** argv) {
    yydebug = 1;

    if (argc > 1) {
        std::istringstream iStringStream = StreamLinesFromFile(argv[1]);
        lexer = new yyFlexLexer(iStringStream, std::cout);
        yyparse();
        auto semantic = Semantic::GetInstance(Root);
        semantic->analyze();
        CreateDotFile(Root);
    }
    else {
        std::cout << "not found file" << std::endl;
    }
}
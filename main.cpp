#include "ast.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"
#include "./utils/io.hpp"

#include "parser.tab.c"

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
        std::unordered_map<std::string, ClassEntity*> classes = {{"$" + Root->packageName, semantic->packageClass}};
        Generator().generate(classes);
    }
    else {
        std::cout << "not found file" << std::endl;
    }
}
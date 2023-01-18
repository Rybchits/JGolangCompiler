#include "ast.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"
#include "./utils/io.hpp"

#include "parser.tab.c"

#include <iostream>
#include <filesystem>

int main(int argc, char** argv) {
    yydebug = 0;

    std::istringstream iStringStream;
    if (argc > 1) {
        iStringStream = StreamLinesFromFile(argv[1]);
    }

    if (argc <= 1 || !iStringStream.rdbuf()->in_avail()) {
        std::cout << "file is empty or not found" << std::endl;
        return 1;
    }

    lexer = new yyFlexLexer(iStringStream, std::cout);
    yyparse();
    auto semantic = Semantic::GetInstance(Root);
    bool isSematicOk = semantic->analyze();
    CreateDotFile(Root);
    
    if (isSematicOk) {
        std::unordered_map<std::string, ClassEntity*> classes = {{ "$" + Root->packageName, semantic->packageClass}};
        Generator(classes, semantic->typesExpressions).generate();
    }

    return 0;
}
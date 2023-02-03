#include "ast.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"

#include "./utils/io.hpp"
#include "./parser/parser.tab.h"

#include <FlexLexer.h>
#include <iostream>
#include <filesystem>

yyFlexLexer* lexer;
PackageAST *Root;

int main(int argc, char** argv) {
    yydebug = 0;    // set 1 to debug bison

    std::istringstream iStringStream;

    if (argc > 1) {
        iStringStream = StreamLinesFromFile(argv[1]);

        if (!iStringStream.rdbuf()->in_avail()) {
            std::cout << "file is empty or not found" << std::endl;
            return 1;
        }

    } else {
        std::cout << "no go files listed" << std::endl;
        return 1;
    }

    lexer = new yyFlexLexer(iStringStream, std::cout);
    yyparse();

    auto semantic = new Semantic(Root);
    bool isSematicOk = semantic->analyze();

    CreateDotFile(Root);
    
    if (isSematicOk) {
        // Only package class
        std::unordered_map<std::string, ClassEntity*> classes = { { "$" + Root->packageName, semantic->packageClass} };
        Generator(classes).generate();
    }

    return 0;
}
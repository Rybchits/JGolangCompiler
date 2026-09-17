#include "ast.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"

#include "./utils/io.hpp"
#include "parser/driver.hpp"

#include <iostream>
#include <filesystem>

int main(int argc, char** argv) {
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

    jgolang::Driver driver;
    auto result = driver.parse(iStringStream, argv[1]);
    if (!result) {
        for (const auto& diagnostic : result.diagnostics) {
            std::cerr << diagnostic.filename << ':' << diagnostic.line << ':'
                      << diagnostic.column << ": Error: " << diagnostic.message << '\n';
        }
        return 1;
    }

    Semantic semantic(result.root.get());
    bool isSematicOk = semantic.analyze();

    CreateDotFile(result.root.get());
    
    if (isSematicOk) {
        // Only package class
        std::unordered_map<std::string, ClassEntity*> classes = { { "$" + result.root->packageName, semantic.packageClass.get()} };
        Generator(classes).generate();
    }

    return isSematicOk ? 0 : 1;
}

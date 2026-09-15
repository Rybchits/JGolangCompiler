#include "parser/driver.hpp"

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: parse_test <source.go> <accept|reject>\n";
        return 1;
    }
    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "Cannot open " << argv[1] << '\n';
        return 1;
    }
    jgolang::Driver driver;
    auto result = driver.parse(input, argv[1]);
    const bool expected = std::string(argv[2]) == "accept";
    
    if (bool(result) != expected || (!expected && result.diagnostics.empty())) {
        std::cerr << "Expected parser to " << argv[2] << " the input\n";
        for (const auto& diagnostic : result.diagnostics) {
            std::cerr << diagnostic.line << ':' << diagnostic.column << ": "
                      << diagnostic.message << '\n';
        }
        return 1;
    }
}

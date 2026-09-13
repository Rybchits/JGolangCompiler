#pragma once

#include "ast.h"

#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace jgolang {

struct Diagnostic {
    std::string filename;
    int line;
    int column;
    std::string message;
};

struct ParseResult {
    std::unique_ptr<PackageAST> root;
    std::vector<Diagnostic> diagnostics;

    explicit operator bool() const { return root && diagnostics.empty(); }
};

class Driver {
public:
    ParseResult parse(std::istream& input, const std::string& filename = "<input>");

    // Called by the parser; ownership of the root passes to the result.
    void setRoot(PackageAST* root);
    void reportError(int line, int column, const std::string& message);

private:
    std::string filename_;
    ParseResult result_;
};

} // namespace jgolang

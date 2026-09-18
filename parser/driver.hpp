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
    PackageASTPtr root;
    std::vector<Diagnostic> diagnostics;

    explicit operator bool() const { return root && diagnostics.empty(); }
};

class Driver {
public:
    ParseResult parse(std::istream& input, const std::string& filename = "<input>");

    void setRoot(PackageASTPtr root);
    void reportError(int line, int column, const std::string& message);

private:
    std::string filename_;
    ParseResult result_;
};

} // namespace jgolang

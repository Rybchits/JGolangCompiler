#include "parser/driver.hpp"
#include "lexer/scanner.hpp"

#include <utility>

namespace jgolang {

ParseResult Driver::parse(std::istream& input, const std::string& filename) {
    filename_ = filename;
    result_ = {};
    Scanner scanner(input, filename);
    Parser parser(*this, scanner);

    const int status = parser.parse();
    if (status != 0 || !result_.diagnostics.empty()) {
        result_.root.reset();
    }
    return std::move(result_);
}

void Driver::setRoot(PackageASTPtr root) {
    result_.root = std::move(root);
}

void Driver::reportError(int line, int column, const std::string& message) {
    result_.diagnostics.push_back({filename_, line, column, message});
}

void Parser::error(const location_type& location, const std::string& message) {
    driver.reportError(location.begin.line, location.begin.column, message);
}

} // namespace jgolang

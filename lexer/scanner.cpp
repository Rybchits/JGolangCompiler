#include "lexer/scanner.hpp"

#include <stdexcept>

namespace jgolang {

Scanner::Scanner(std::istream& input, const std::string& filename)
    : yyFlexLexer(&input), filename_(filename) {
    location_.initialize(&filename_);
}

int Scanner::lex(Parser::semantic_type* value, Parser::location_type* location) {
    value_ = value;
    try {
        const int token = yylex();
        if (token == 0) {
            location_.step();
        }
        *location = location_;
        return token;
    } catch (const std::out_of_range&) {
        fail("numeric literal is out of range");
    } catch (const std::invalid_argument&) {
        fail("invalid numeric literal");
    }
}

void Scanner::advanceLocation(const char* text, int length) {
    location_.step();
    for (int i = 0; i < length; ++i) {
        if (text[i] == '\n') {
            location_.lines(1);
        } else {
            location_.columns(1);
        }
    }
}

void Scanner::markCompositeLiteralInHeader() {
    if (insideHeaderConstruct && nestingBracketsAtHeaderConstruct == 0) {
        isCompositeLiteralAtHeaderConstruct = true;
    }
}

void Scanner::finishCompositeLiteral() {
    isCompositeLiteralAtHeaderConstruct = false;
}

void Scanner::fail(const std::string& message) const {
    throw Parser::syntax_error(location_, message);
}

void Scanner::LexerError(const char* message) {
    fail(message);
}

} // namespace jgolang

int yylex(jgolang::Parser::semantic_type* value,
          jgolang::Parser::location_type* location,
          jgolang::Scanner& scanner) {
    return scanner.lex(value, location);
}

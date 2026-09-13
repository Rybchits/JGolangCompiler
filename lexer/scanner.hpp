#pragma once

#include <parser/parser.tab.h>

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

#include <istream>
#include <string>

namespace jgolang {

class Scanner : public yyFlexLexer {
public:
    Scanner(std::istream& input, const std::string& filename);
    int lex(Parser::semantic_type* value, Parser::location_type* location);

    void markCompositeLiteralInHeader();
    void finishCompositeLiteral();

private:
    int yylex() override;
    void LexerError(const char* message) override;
    void advanceLocation(const char* text, int length);
    [[noreturn]] void fail(const std::string& message) const;

    std::string filename_;
    Parser::semantic_type* value_ = nullptr;
    Parser::location_type location_;
    Parser::location_type literalLocation_;

    bool insideHeaderConstruct = false;
    bool isCompositeLiteralAtHeaderConstruct = false;
    int nestingBracketsAtHeaderConstruct = 0;

    std::string stringLiteral;
    std::string currentComment;
    int32_t runeValue = 0;
    long long currentInteger = 0;
    double floatingPoint = 0;
};

} // namespace jgolang

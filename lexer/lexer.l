%option noyywrap
%option never-interactive
%option c++
%option yyclass="jgolang::Scanner"
%option yylineno
%option stack

%{
  #include "lexer/scanner.hpp"
  #include "lexer.hpp"

  using yytokentype = jgolang::Parser::token;
  #define YY_USER_ACTION advanceLocation(yytext, yyleng);
%}


LETTER                  [A-Za-z_]
DECIMAL_DIGIT           [0-9]
DECIMAL_DIGITS          {DECIMAL_DIGIT}(_?{DECIMAL_DIGIT})*

HEX_DIGIT               [0-9A-Fa-f]
HEX_DIGITS              {HEX_DIGIT}(_?{HEX_DIGIT})*

OCT_DIGIT               [0-7]
BIN_DIGIT               [01]

DECIMAL_EXPONENT        [eE][+-]?{DECIMAL_DIGITS}
HEX_EXPONENT            [pP][+-]?{DECIMAL_DIGITS}

DECIMAL_FLOAT_LITERAL   {DECIMAL_DIGITS}\.{DECIMAL_DIGITS}?{DECIMAL_EXPONENT}?|{DECIMAL_DIGITS}{DECIMAL_EXPONENT}|\.{DECIMAL_DIGITS}{DECIMAL_EXPONENT}?

HEX_MANTISSA            (_?{HEX_DIGITS}\.{HEX_DIGITS}?|_?{HEX_DIGITS}|\.{HEX_DIGITS})
HEX_FLOAT_LITERAL       0[xX]{HEX_MANTISSA}{HEX_EXPONENT}


HEX_BYTE_VALUE          \\x{HEX_DIGIT}{2}
OCTAL_BYTE_VALUE        \\{OCT_DIGIT}{3}


%x MULTILINE_COMMENT
%x STRING_LITERAL
%x RUNE_LITERAL
%x ENABLE_SEMICOLON

%%


import                                  { PrintKeyword(yytext); }
select                                  { PrintKeyword(yytext); }
defer                                   { PrintKeyword(yytext); }
go                                      { PrintKeyword(yytext); }
chan                                    { PrintKeyword(yytext); }
goto                                    { PrintKeyword(yytext); }

interface                               { return yytokentype::INTERFACE;     }
default                                 { return yytokentype::DEFAULT;       }
func                                    { return yytokentype::FUNC;          }
case                                    { return yytokentype::CASE;          }
map                                     { return yytokentype::MAP;           }
else                                    { return yytokentype::ELSE;          }
package                                 { return yytokentype::PACKAGE;       }
const                                   { return yytokentype::CONST;         }
range                                   { return yytokentype::RANGE;         }
type                                    { return yytokentype::TYPE;          }
var                                     { return yytokentype::VAR;           }

if                                      { return yytokentype::IF;      }
for                                     { return yytokentype::FOR;     }
switch                                  { return yytokentype::SWITCH;  }

nil                                     { yy_push_state(ENABLE_SEMICOLON); return yytokentype::NIL;          }
false                                   { yy_push_state(ENABLE_SEMICOLON); return yytokentype::FALSE;        }
true                                    { yy_push_state(ENABLE_SEMICOLON); return yytokentype::TRUE;         }
break                                   { yy_push_state(ENABLE_SEMICOLON); return yytokentype::BREAK;        }
fallthrough                             { yy_push_state(ENABLE_SEMICOLON); return yytokentype::FALLTHROUGH;  }
continue                                { yy_push_state(ENABLE_SEMICOLON); return yytokentype::CONTINUE;     }
return                                  { yy_push_state(ENABLE_SEMICOLON); return yytokentype::RETURN;       }

struct                                  { return yytokentype::STRUCT; }


"/*"                                    { currentComment.clear(); yy_push_state(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>\*+\/                { yy_pop_state();                                           }
<MULTILINE_COMMENT>[^\*\/]+             { currentComment += yytext;                                 }
<MULTILINE_COMMENT><<EOF>>              { fail("expected */ "); yy_pop_state();        }

\/\/.*                                  { std::string tempStr = yytext;                             }


\"                                      { stringLiteral.clear(); literalLocation_ = location_; yy_push_state(STRING_LITERAL);     }
<STRING_LITERAL>{OCTAL_BYTE_VALUE}      { stringLiteral += char(std::stoi(yytext + 1, nullptr, 8)); }
<STRING_LITERAL>{HEX_BYTE_VALUE}        { stringLiteral += char(std::stoi(yytext + 2, nullptr, 16));}
<STRING_LITERAL>[^\\\"\n#{}]+           { stringLiteral += yytext;  }
<STRING_LITERAL>\\a                     { stringLiteral += "\a";    }
<STRING_LITERAL>\\b                     { stringLiteral += "\b";    }
<STRING_LITERAL>\\\\                    { stringLiteral += "\\";    }
<STRING_LITERAL>\\t                     { stringLiteral += "\t";    }
<STRING_LITERAL>\\n                     { stringLiteral += "\n";    }
<STRING_LITERAL>\\f                     { stringLiteral += "\f";    }
<STRING_LITERAL>\\r                     { stringLiteral += "\r";    }
<STRING_LITERAL>\\v                     { stringLiteral += "\v";    }
<STRING_LITERAL>\\\"                    { stringLiteral += "\"";    }
<STRING_LITERAL>\\.                     { fail("unknown escape sequence"); yy_pop_state();}
<STRING_LITERAL>\n                      { fail("newline in string"); yy_pop_state();      }
<STRING_LITERAL><<EOF>>                 { fail("expected \" "); yy_pop_state();           }

<STRING_LITERAL>\"                      {   location_.begin = literalLocation_.begin; value_->emplace<std::string>(stringLiteral);
                                            yy_push_state(ENABLE_SEMICOLON);
                                            return yytokentype::STRING_LIT;
                                        }

<ENABLE_SEMICOLON>[" "\t]+              { }
<ENABLE_SEMICOLON>\/\/.*                { std::string tempStr = yytext; }
<ENABLE_SEMICOLON>"/*"                  { currentComment.clear(); yy_push_state(MULTILINE_COMMENT); }
<ENABLE_SEMICOLON><<EOF>>               { BEGIN(INITIAL); return ';'; }
<ENABLE_SEMICOLON>\n+                   { BEGIN(INITIAL); return ';'; }
<ENABLE_SEMICOLON>""/[^\n]              { BEGIN(INITIAL); }




\'                                      { runeValue = -1; literalLocation_ = location_; yy_push_state(RUNE_LITERAL); }

<RUNE_LITERAL>{OCTAL_BYTE_VALUE}        { if (runeValue == -1) runeValue = std::stoi(yytext + 1, nullptr, 8);
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>{HEX_BYTE_VALUE}          { if (runeValue == -1) runeValue = std::stoi(yytext + 1, nullptr, 16);   
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>[^'\n\\]                  { if (runeValue == -1) runeValue = *yytext;                              
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\a                       { if (runeValue == -1) runeValue = '\a';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\b                       { if (runeValue == -1) runeValue = '\b';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\\\                      { if (runeValue == -1) runeValue = '\\';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\t                       { if (runeValue == -1) runeValue = '\t';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\n                       { if (runeValue == -1) runeValue = '\n';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\f                       { if (runeValue == -1) runeValue = '\f';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\r                       { if (runeValue == -1) runeValue = '\r';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\v                       { if (runeValue == -1) runeValue = '\v';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\\'                      { if (runeValue == -1) runeValue = '\'';                                 
                                          else fail("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\.                       { fail("unknown escape sequence");    }
<RUNE_LITERAL>\n                        { fail("newline in string");          }
<RUNE_LITERAL><<EOF>>                   { fail("expected \" ");               }

<RUNE_LITERAL>\'                        { if (runeValue == -1) fail("empty rune literal");
                                          location_.begin = literalLocation_.begin; value_->emplace<int32_t>(runeValue);
                                          yy_pop_state();
                                          return yytokentype::RUNE_LIT;
                                        }


"&&"                                    { return yytokentype::AND;   }
"||"                                    { return yytokentype::OR;    }
"++"                                    { yy_push_state(ENABLE_SEMICOLON); return yytokentype::INCREMENT; }
"--"                                    { yy_push_state(ENABLE_SEMICOLON); return yytokentype::DECREMENT; }

"&="                                    { PrintAssignmentOperator("&=");    }
"&^="                                   { PrintAssignmentOperator("&^=");   }
"|="                                    { PrintAssignmentOperator("|=");    }
"^="                                    { PrintAssignmentOperator("^=");    }
"<<="                                   { PrintAssignmentOperator("<<=");   }
">>="                                   { PrintAssignmentOperator(">>=");   }

"&"                                     { PrintArithmeticOperator("&");     }
"&^"                                    { PrintArithmeticOperator("&^");    }
"|"                                     { PrintArithmeticOperator("|");     }
"^"                                     { PrintArithmeticOperator("^");     }
"<<"                                    { PrintArithmeticOperator("<<");    }
">>"                                    { PrintArithmeticOperator(">>");    }

">="                                    { return yytokentype::GREATER_OR_EQUAL;          }
"<="                                    { return yytokentype::LESS_OR_EQUAL;             }
":="                                    { return yytokentype::SHORT_DECL_OP;             }
"..."                                   { return yytokentype::VARIADIC;                  }

"+="                                    { return yytokentype::PLUS_ASSIGNMENT;           }
"-="                                    { return yytokentype::MINUS_ASSIGNMENT;          }
"*="                                    { return yytokentype::MUL_ASSIGNMENT;            }
"/="                                    { return yytokentype::DIV_ASSIGNMENT;            }
"%="                                    { return yytokentype::MOD_ASSIGNMENT;            }
"!="                                    { return yytokentype::NOT_EQUAL;                 }
"=="                                    { return yytokentype::EQUAL;                     }

"!"                                     { return yytext[0]; }
">"                                     { return yytext[0]; }
"<"                                     { return yytext[0]; }
"~"                                     { return yytext[0]; }
"="                                     { return yytext[0]; }
"+"                                     { return yytext[0]; }
"-"                                     { return yytext[0]; }
"*"                                     { return yytext[0]; }
"/"                                     { return yytext[0]; }
"%"                                     { return yytext[0]; }
"["                                     { return yytext[0]; }
"."                                     { return yytext[0]; }
","                                     { return yytext[0]; }
";"                                     { return yytext[0]; }
":"                                     { return yytext[0]; }
"}"                                     { yy_push_state(ENABLE_SEMICOLON); return yytext[0]; }
"]"                                     { yy_push_state(ENABLE_SEMICOLON); return yytext[0]; }

")"                                     { yy_push_state(ENABLE_SEMICOLON); return ')'; }
"{"                                     { return '{'; }
"("                                     { return '('; }


{DECIMAL_DIGITS}+                       {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr);

                                          value_->emplace<long long>(currentInteger);
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

0[oO](_?{OCT_DIGIT})+                   {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr.substr(2), nullptr, 8);

                                          value_->emplace<long long>(currentInteger);
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

0[xX](_?{HEX_DIGIT})+                   {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr, nullptr, 16);

                                          value_->emplace<long long>(currentInteger);
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

0[bB](_?{BIN_DIGIT})+                   {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr.substr(2), nullptr, 2);

                                          value_->emplace<long long>(currentInteger);
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

{DECIMAL_FLOAT_LITERAL}|{HEX_FLOAT_LITERAL} {
                                                std::string tempStr = yytext;
                                                tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                                floatingPoint = std::stod(tempStr.c_str());

                                                value_->emplace<double>(floatingPoint);
                                                yy_push_state(ENABLE_SEMICOLON);
                                                return yytokentype::FLOAT_LIT;
                                            }


({LETTER})({LETTER}|{DECIMAL_DIGIT})*   {
                                          value_->emplace<std::string>(yytext, yyleng);
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::IDENTIFIER;
                                        }

[[:space:]]+                            { }

.                                       { fail("found bad character"); }

%option noyywrap
%option never-interactive
%option c++
%option yylineno
%option stack

%{
  #include "../ast.h"
  #include "lexer.hpp"

  // shared variables for lexer and parser 
  bool insideHeaderConstruct;
  bool isCompositeLiteralAtHeaderConstruct;
  int nestingBracketsAtHeaderConstruct;
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
%x EDGE_OF_CONSTRUCTION

%%

%{
std::string stringLiteral;
std::string currentComment;
int32_t runeValue;
long long currentInteger;
double floatingPoint;
%}

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

if                                      { yy_push_state(EDGE_OF_CONSTRUCTION); return yytokentype::IF;      }
for                                     { yy_push_state(EDGE_OF_CONSTRUCTION); return yytokentype::FOR;     }
switch                                  { yy_push_state(EDGE_OF_CONSTRUCTION); return yytokentype::SWITCH;  }

nil                                     { yy_push_state(ENABLE_SEMICOLON); return yytokentype::NIL;          }
false                                   { yy_push_state(ENABLE_SEMICOLON); return yytokentype::FALSE;        }
true                                    { yy_push_state(ENABLE_SEMICOLON); return yytokentype::TRUE;         }
break                                   { yy_push_state(ENABLE_SEMICOLON); return yytokentype::BREAK;        }
fallthrough                             { yy_push_state(ENABLE_SEMICOLON); return yytokentype::FALLTHROUGH;  }
continue                                { yy_push_state(ENABLE_SEMICOLON); return yytokentype::CONTINUE;     }
return                                  { yy_push_state(ENABLE_SEMICOLON); return yytokentype::RETURN;       }

struct                                  {
                                          if (insideHeaderConstruct && nestingBracketsAtHeaderConstruct == 0) {
                                            isCompositeLiteralAtHeaderConstruct = true;
                                          }
                                          return yytokentype::STRUCT;        
                                        }


"/*"                                    { currentComment.clear(); yy_push_state(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>\*+\/                { yy_pop_state();                                           }
<MULTILINE_COMMENT>[^\*\/]+             { currentComment += yytext;                                 }
<MULTILINE_COMMENT><<EOF>>              { PrintErrorAndExit("expected */ "); yy_pop_state();        }

\/\/.*                                  { std::string tempStr = yytext;                             }


\"                                      { stringLiteral.clear(); yy_push_state(STRING_LITERAL);     }
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
<STRING_LITERAL>\\.                     { PrintErrorAndExit("unknown escape sequence"); yy_pop_state();}
<STRING_LITERAL>\n                      { PrintErrorAndExit("newline in string"); yy_pop_state();      }
<STRING_LITERAL><<EOF>>                 { PrintErrorAndExit("expected \" "); yy_pop_state();           }

<STRING_LITERAL>\"                      {   yylval.stringVal = DuplicateString(stringLiteral.c_str());
                                            yy_push_state(ENABLE_SEMICOLON);
                                            return yytokentype::STRING_LIT;
                                        }

<ENABLE_SEMICOLON>[" "\t]+              { }
<ENABLE_SEMICOLON>\/\/.*                { std::string tempStr = yytext; }
<ENABLE_SEMICOLON>"/*"                  { currentComment.clear(); yy_push_state(MULTILINE_COMMENT); }
<ENABLE_SEMICOLON><<EOF>>               { BEGIN(INITIAL); return ';'; }
<ENABLE_SEMICOLON>\n+                   { BEGIN(INITIAL); return ';'; }
<ENABLE_SEMICOLON>""/[^\n]              { BEGIN(INITIAL); }


<EDGE_OF_CONSTRUCTION>""/.              { yy_pop_state();
                                          insideHeaderConstruct = !insideHeaderConstruct;
                                          return insideHeaderConstruct? '(' : '{';
                                        }

<EDGE_OF_CONSTRUCTION>[[:space:]]+      { }


\'                                      { runeValue = -1; yy_push_state(RUNE_LITERAL); }

<RUNE_LITERAL>{OCTAL_BYTE_VALUE}        { if (runeValue == -1) runeValue = std::stoi(yytext + 1, nullptr, 8);
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>{HEX_BYTE_VALUE}          { if (runeValue == -1) runeValue = std::stoi(yytext + 1, nullptr, 16);   
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>[^'\n\\]                  { if (runeValue == -1) runeValue = *yytext;                              
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\a                       { if (runeValue == -1) runeValue = '\a';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\b                       { if (runeValue == -1) runeValue = '\b';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\\\                      { if (runeValue == -1) runeValue = '\\';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\t                       { if (runeValue == -1) runeValue = '\t';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\n                       { if (runeValue == -1) runeValue = '\n';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\f                       { if (runeValue == -1) runeValue = '\f';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\r                       { if (runeValue == -1) runeValue = '\r';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\v                       { if (runeValue == -1) runeValue = '\v';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\\'                      { if (runeValue == -1) runeValue = '\'';                                 
                                          else PrintErrorAndExit("more than one character in rune literal");
                                        }

<RUNE_LITERAL>\\.                       { PrintErrorAndExit("unknown escape sequence");    }
<RUNE_LITERAL>\n                        { PrintErrorAndExit("newline in string");          }
<RUNE_LITERAL><<EOF>>                   { PrintErrorAndExit("expected \" ");               }

<RUNE_LITERAL>\'                        { if (runeValue == -1) PrintErrorAndExit("empty rune literal");
                                          yylval.runeVal = runeValue;
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

")"                                     {
                                          if (insideHeaderConstruct) {
                                            nestingBracketsAtHeaderConstruct--;
                                          }
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytext[0];
                                        }

"{"                                     {
                                          if (insideHeaderConstruct 
                                              && nestingBracketsAtHeaderConstruct == 0
                                              && !isCompositeLiteralAtHeaderConstruct
                                          ) {
                                              yy_push_state(EDGE_OF_CONSTRUCTION);
                                              return ')';
                                          }
                                          return yytext[0]; 
                                        }

"("                                     { 
                                          if (insideHeaderConstruct) {
                                            nestingBracketsAtHeaderConstruct++;
                                          }
                                          return yytext[0];
                                        }


{DECIMAL_DIGITS}+                       {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr);

                                          yylval.integerVal = currentInteger;
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

0[oO](_?{OCT_DIGIT})+                   {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr.substr(2), nullptr, 8);

                                          yylval.integerVal = currentInteger;
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

0[xX](_?{HEX_DIGIT})+                   {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr, nullptr, 16);

                                          yylval.integerVal = currentInteger;
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

0[bB](_?{BIN_DIGIT})+                   {
                                          std::string tempStr = yytext;
                                          tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                          currentInteger = std::stoll(tempStr.substr(2), nullptr, 2);

                                          yylval.integerVal = currentInteger;
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::INT_LIT;
                                        }

{DECIMAL_FLOAT_LITERAL}|{HEX_FLOAT_LITERAL} {
                                                std::string tempStr = yytext;
                                                tempStr.erase(std::remove(tempStr.begin(), tempStr.end(), '_'), tempStr.end());
                                                floatingPoint = std::stod(tempStr.c_str());

                                                yylval.floatVal = floatingPoint;
                                                yy_push_state(ENABLE_SEMICOLON);
                                                return yytokentype::FLOAT_LIT;
                                            }


({LETTER})({LETTER}|{DECIMAL_DIGIT})*   {
                                          yylval.identifierVal = DuplicateString(yytext);
                                          yy_push_state(ENABLE_SEMICOLON);
                                          return yytokentype::IDENTIFIER;
                                        }

[[:space:]]+                            { }

.                                       { PrintErrorAndExit("found bad character"); }


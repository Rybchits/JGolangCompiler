#include <iostream>
#include <fstream>
#include <sstream>

#include "parser.cpp"
#include "ast.h"

int main(int argc, char** argv) {
    yydebug = 1;

    if (argc > 1) {
        std::ifstream ifile;

        ifile.open(argv[1]);

        std::string strSum;

        while (!ifile.eof()){
            std::string str;
            getline(ifile, str);
            strSum += str + '\n';
        }

        std::istringstream strStream(strSum);
        lexer = new yyFlexLexer(strStream, std::cout);
        yyparse();

        std::cout << Root->name() << std::endl;
        ifile.close();
    }
    else {
        std::cout << "not found file" << std::endl;
    }
}
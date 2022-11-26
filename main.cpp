#include <iostream>

#include "tree_nodes.h"

extern PackageAST *Root;
extern int yyparse();
extern FILE* yyin;


int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        yyparse();
        fclose(yyin);
    }
    else {
        std::cout << "not found file" << std::endl;
    }
}
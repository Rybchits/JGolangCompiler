#include <iostream>
#include <fstream>
#include <sstream>

#include "../ast.h"
#include "./dot_visitor.h"

using namespace std::filesystem;

std::istringstream StreamLinesFromFile(const std::string& filepath) {
    std::ifstream file;
    file.open(filepath);

    std::string fileContent;

    while (!file.eof()){
        std::string str;
        getline(file, str);
        fileContent += str + '\n';
    }

    file.close();
    return std::istringstream(fileContent);
}

bool CreateDotFile(NodeAST* node) {
    const auto dotFile = current_path() / "output" / "tree.dot";

    create_directory(current_path() / "output");
    {
        std::fstream treeOut;
        treeOut.open(dotFile, std::ios_base::out);

        auto dotVisitor = new DotConvertVisitor(treeOut);
        dotVisitor->convert(node);
    }

    return true;
}

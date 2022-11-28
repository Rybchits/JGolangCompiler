#include <iostream>
#include <fstream>
#include <sstream>


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

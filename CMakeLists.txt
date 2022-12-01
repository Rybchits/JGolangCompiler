cmake_minimum_required(VERSION 3.23)
project(JGolangCompiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(JGolangCompiler main.cpp ast.cpp ast.h enums.h parser.hpp parser.cpp lexer.cpp utils/io.cpp utils/dot.cpp)

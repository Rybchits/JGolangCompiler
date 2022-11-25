cmake_minimum_required(VERSION 3.23)
project(JGolangCompiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(JGolangCompiler main.cpp tree_nodes.cpp tree_nodes.h enums.h parser.hpp parser.cpp)

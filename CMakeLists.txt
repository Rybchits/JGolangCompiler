cmake_minimum_required(VERSION 3.23)
project(JGolangCompiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(JGolangCompiler main.cpp ast.cpp ast.h enums.h parser.hpp parser.cpp lexer.cpp utils/io.cpp semantic/semantic.cpp semantic/semantic.h semantic/java_class.cpp semantic/java_class.h semantic/java_method.cpp semantic/java_method.h semantic/java_interface.cpp semantic/java_interface.h  utils/dot_visitor.cpp utils/dot_visitor.h visitor.h)

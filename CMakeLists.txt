cmake_minimum_required(VERSION 3.15)
project(JGolangCompiler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLEX REQUIRED)
find_package(BISON 3.8.2 REQUIRED)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/parser
    ${CMAKE_CURRENT_BINARY_DIR}/lexer
)

BISON_TARGET(Parser
    parser/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.tab.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser.tab.h
    COMPILE_FLAGS "-t --verbose"
)

FLEX_TARGET(Lexer
    lexer/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lexer/lex.yy.cc
)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

add_library(compiler_core STATIC
    ast.cpp
    parser/driver.cpp
    lexer/scanner.cpp
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
    entities/class_entity.cpp
    entities/type_entity.cpp
    semantic/precalculate_visitor.cpp
    semantic/semantic.cpp
    semantic/statements_visitor.cpp
    semantic/types_visitor.cpp
    utils/bytes.cpp
    utils/dot_visitor.cpp
    codegen/codegen.cpp
    codegen/constant.cpp
)

target_include_directories(compiler_core PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer
)

add_executable(compiler main.cpp)
target_link_libraries(compiler PRIVATE compiler_core)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

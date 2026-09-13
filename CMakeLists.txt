cmake_minimum_required(VERSION 3.15)
project(JGolangCompiler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

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

add_executable(compiler
    main.cpp
    ast.cpp
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

target_include_directories(compiler PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer
)

include(CTest)

if(BUILD_TESTING)
    set(ONLY_MAIN_TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/tests/only_main")
    file(MAKE_DIRECTORY "${ONLY_MAIN_TEST_DIR}")

    add_test(
        NAME only_main_unused_variable
        COMMAND compiler "${CMAKE_CURRENT_SOURCE_DIR}/tests/functions/only_main.go"
    )
    set_tests_properties(only_main_unused_variable PROPERTIES
        WORKING_DIRECTORY "${ONLY_MAIN_TEST_DIR}"
        PASS_REGULAR_EXPRESSION "Error: Unused variable myVar"
        TIMEOUT 10
    )
endif()

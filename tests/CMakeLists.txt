add_executable(parse_test runner/parse_test.cpp)
target_link_libraries(parse_test PRIVATE compiler_core)

foreach(case_name IN ITEMS basic parenthesized call index arrays function nested_function)
    add_test(NAME header_${case_name}
        COMMAND parse_test "${CMAKE_CURRENT_SOURCE_DIR}/cases/headers/${case_name}.go" accept)
    set_tests_properties(header_${case_name} PROPERTIES TIMEOUT 10 LABELS "parser;headers")
endforeach()

add_test(NAME header_ambiguous
    COMMAND parse_test "${CMAKE_CURRENT_SOURCE_DIR}/cases/headers/ambiguous.go" reject)
set_tests_properties(header_ambiguous PROPERTIES TIMEOUT 10 LABELS "parser;headers")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(Java 18 REQUIRED COMPONENTS Runtime Development)
find_program(GO_EXECUTABLE go)
find_program(DIFF_EXECUTABLE diff)
if(NOT GO_EXECUTABLE OR NOT DIFF_EXECUTABLE)
    message(FATAL_ERROR "Algorithm tests require go and diff")
endif()

foreach(case_name IN ITEMS binary-search bubble-sort factorial heap-sort square_paint square_snake)
    add_test(NAME algorithm_${case_name}
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/runner/compare_go.py"
            --source "${CMAKE_CURRENT_SOURCE_DIR}/cases/algorithms/${case_name}.go"
            --work-dir "${CMAKE_CURRENT_BINARY_DIR}/algorithms/${case_name}"
            --compiler "$<TARGET_FILE:compiler>"
            --go "${GO_EXECUTABLE}"
            --java "${Java_JAVA_EXECUTABLE}"
            --javac "${Java_JAVAC_EXECUTABLE}"
            --diff "${DIFF_EXECUTABLE}"
            --runtime "${PROJECT_SOURCE_DIR}/RTL/$Base.java")
    set_tests_properties(algorithm_${case_name} PROPERTIES TIMEOUT 180 LABELS "runtime;algorithms")
endforeach()

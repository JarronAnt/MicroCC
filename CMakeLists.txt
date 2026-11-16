
cmake_minimum_required(VERSION 3.20)

project(
    MicroCC
    VERSION 0.2.0
    DESCRIPTION "A tiny educational C compiler written in modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Compiler library
# ============================================================

add_library(microcc_core STATIC
    src/Lexer.cpp
    src/Parser.cpp
    src/codegen/AArch64MacOS.cpp
)

target_include_directories(microcc_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# ============================================================
# Compiler executable
# ============================================================

add_executable(microcc
    src/main.cpp
)

target_link_libraries(microcc
    PRIVATE microcc_core
)

# ============================================================
# Compiler warnings
# ============================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(microcc_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    target_compile_options(microcc PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

if(NOT APPLE)
    message(WARNING
        "MicroCC currently emits AArch64 macOS assembly. "
        "End-to-end tests require an Apple Silicon Mac."
    )
endif()

# ============================================================
# Testing
# ============================================================

enable_testing()

function(add_microcc_return_test name expected phase)
    add_test(
        NAME ${phase}.${name}
        COMMAND
            ${CMAKE_COMMAND}
            -DMICROCC=$<TARGET_FILE:microcc>
            -DSOURCE=${PROJECT_SOURCE_DIR}/tests/integration/${name}.c
            -DOUTPUT=${PROJECT_BINARY_DIR}/${phase}_${name}
            -DEXPECTED_EXIT=${expected}
            -P ${PROJECT_SOURCE_DIR}/tests/cmake/run_return_test.cmake
    )
endfunction()

function(add_microcc_failure_test name diagnostic)
    add_test(
        NAME phase2b.error_${name}
        COMMAND
            ${CMAKE_COMMAND}
            -DMICROCC=$<TARGET_FILE:microcc>
            -DSOURCE=${PROJECT_SOURCE_DIR}/tests/integration/${name}.c
            -DOUTPUT=${PROJECT_BINARY_DIR}/phase2b_error_${name}.s
            -DEXPECTED_DIAGNOSTIC=${diagnostic}
            -P ${PROJECT_SOURCE_DIR}/tests/cmake/run_failure_test.cmake
    )
endfunction()

# ============================================================
# Phase 1
# ============================================================

add_microcc_return_test(return_42 42 phase1)

# ============================================================
# Phase 2A
# ============================================================

add_microcc_return_test(precedence 14 phase2a)
add_microcc_return_test(parentheses 20 phase2a)
add_microcc_return_test(associativity 4 phase2a)
add_microcc_return_test(unary 13 phase2a)
add_microcc_return_test(division 5 phase2a)
add_microcc_return_test(nesting 10 phase2a)

# ============================================================
# Phase 2B
# ============================================================

add_microcc_return_test(modulo 8 phase2b)
add_microcc_return_test(comparisons 6 phase2b)
add_microcc_return_test(equality 3 phase2b)
add_microcc_return_test(bitwise 14 phase2b)
add_microcc_return_test(bitwise_precedence 7 phase2b)
add_microcc_return_test(shifts 32 phase2b)
add_microcc_return_test(shift_precedence 2 phase2b)
add_microcc_return_test(logical 2 phase2b)
add_microcc_return_test(short_circuit 1 phase2b)
add_microcc_return_test(ternary 23 phase2b)
add_microcc_return_test(ternary_nested 7 phase2b)
add_microcc_return_test(ternary_middle_comma 3 phase2b)
add_microcc_return_test(comma 14 phase2b)
add_microcc_return_test(bitwise_not 11 phase2b)
add_microcc_return_test(unary_chain 1 phase2b)
add_microcc_return_test(comprehensive 24 phase2b)

# ============================================================
# Negative tests: expected compilation failures
# ============================================================

add_microcc_failure_test(
    missing_colon
    "expected ':'"
)

add_microcc_failure_test(
    missing_parenthesis
    "expected ')'"
)

add_microcc_failure_test(
    assignment_not_supported
    "assignment '=' is not supported"
)


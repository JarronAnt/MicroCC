cmake_minimum_required(VERSION 3.20)

project(
    MicroCC
    VERSION 0.1.0
    DESCRIPTION "A tiny educational C compiler written in modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(microcc_core STATIC
    src/Lexer.cpp
    src/Parser.cpp
    src/codegen/AArch64MacOS.cpp
)

target_include_directories(microcc_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(microcc
    src/main.cpp
)

target_link_libraries(microcc
    PRIVATE
        microcc_core
)

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
        "MicroCC Phase 1 currently emits AArch64 macOS assembly. "
        "Additional backends will be added later."
    )
endif()

enable_testing()

add_test(
    NAME phase1.return_42
    COMMAND
        ${CMAKE_COMMAND}
        -DMICROCC=$<TARGET_FILE:microcc>
        -DSOURCE=${PROJECT_SOURCE_DIR}/tests/integration/return_42.c
        -DOUTPUT=${PROJECT_BINARY_DIR}/phase1_return_42
        -P ${PROJECT_SOURCE_DIR}/tests/cmake/run_return_test.cmake
)

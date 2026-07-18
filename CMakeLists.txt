cmake_minimum_required(VERSION 3.16)
project(sysy2026_compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SYSY_ENABLE_COVERAGE "Enable Clang source coverage" OFF)

set(SYSY_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/generated)
add_library(sysy_compiler_core STATIC
  src/compiler.cpp
  src/ir.cpp
  src/frontend/parse.cpp
  ${SYSY_GENERATED_DIR}/sysy_parser.cpp
  ${SYSY_GENERATED_DIR}/sysy_scanner.cpp
  src/rv64.cpp)
target_include_directories(sysy_compiler_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(sysy_compiler_core PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
  ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(sysy_compiler_core PRIVATE -Wall -Wextra -Wpedantic)

if(SYSY_ENABLE_COVERAGE)
  target_compile_options(sysy_compiler_core PUBLIC -fprofile-instr-generate -fcoverage-mapping)
  target_link_options(sysy_compiler_core PUBLIC -fprofile-instr-generate -fcoverage-mapping)
endif()

add_executable(compiler src/main.cpp)
target_link_libraries(compiler PRIVATE sysy_compiler_core)

enable_testing()
add_executable(sysy_unit_tests tests/test_compiler.cpp)
target_link_libraries(sysy_unit_tests PRIVATE sysy_compiler_core)
target_include_directories(sysy_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME sysy_unit_tests COMMAND sysy_unit_tests)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME sysy_cli_tests
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cli.py
                 $<TARGET_FILE:compiler>)

find_program(SYSY_BISON_EXECUTABLE NAMES bison PATHS /opt/homebrew/opt/bison/bin)
find_program(SYSY_FLEX_EXECUTABLE NAMES flex)
if(SYSY_BISON_EXECUTABLE AND SYSY_FLEX_EXECUTABLE)
  add_custom_target(regenerate_frontend
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SYSY_GENERATED_DIR}
    COMMAND ${SYSY_BISON_EXECUTABLE}
            --defines=${SYSY_GENERATED_DIR}/sysy_parser.hpp
            --output=${SYSY_GENERATED_DIR}/sysy_parser.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/parser.y
    COMMAND ${SYSY_FLEX_EXECUTABLE}
            --outfile=${SYSY_GENERATED_DIR}/sysy_scanner.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/scanner.l
    VERBATIM)
endif()

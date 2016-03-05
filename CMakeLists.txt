#
# Copyright 2016 WebAssembly Community Group participants
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 2.6)
project(SEXPR_WASM)

if (${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
  set(COMPILER_IS_CLANG 1)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 1)
  set(COMPILER_IS_MSVC 0)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 1)
else ()
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
endif ()

include(CheckSymbolExists)
include(CheckIncludeFile)

check_include_file("alloca.h" HAVE_ALLOCA_H)
check_include_file("getopt.h" HAVE_GETOPT_H)
check_include_file("unistd.h" HAVE_UNISTD_H)

configure_file(
  ${SEXPR_WASM_SOURCE_DIR}/src/wasm-config.h.in
  ${SEXPR_WASM_BINARY_DIR}/wasm-config.h
)

include_directories(src ${SEXPR_WASM_BINARY_DIR})

add_definitions(-Wall -Werror -g)

find_package(FLEX)
find_package(BISON)
if (FLEX_FOUND AND BISON_FOUND)
  set(BISON_PARSER_C ${SEXPR_WASM_BINARY_DIR}/wasm-bison-parser.c)
  set(BISON_PARSER_H )
  BISON_TARGET(WASM_BISON_PARSER
    ${SEXPR_WASM_SOURCE_DIR}/src/wasm-bison-parser.y
    ${BISON_PARSER_C}
    COMPILE_FLAGS --defines=${SEXPR_WASM_BINARY_DIR}/wasm-bison-parser.h
  )

  set(FLEX_LEXER_C ${SEXPR_WASM_BINARY_DIR}/wasm-flex-lexer.c)
  FLEX_TARGET(WASM_FLEX_LEXER
    ${SEXPR_WASM_SOURCE_DIR}/src/wasm-flex-lexer.l
    ${FLEX_LEXER_C}
  )
else ()
  set(BISON_PARSER_C src/prebuilt/wasm-bison-parser.c)
  set(FLEX_LEXER_C src/prebuilt/wasm-flex-lexer.c)
endif ()

set_source_files_properties(
  ${FLEX_LEXER_C} PROPERTIES COMPILE_FLAGS -Wno-unused-function
)

set(SEXPR_WASM_SRCS
  src/sexpr-wasm.c
  src/wasm.c
  src/wasm-allocator.c
  src/wasm-ast.c
  src/wasm-binary-writer.c
  src/wasm-check.c
  src/wasm-stack-allocator.c
  src/wasm-vector.c
  src/wasm-writer.c
  ${BISON_PARSER_C}
  ${FLEX_LEXER_C}
)

add_executable(sexpr-wasm ${SEXPR_WASM_SRCS})
add_custom_target(everything DEPENDS sexpr-wasm)

find_package(PythonInterp)
set(RUN_TESTS_PY ${SEXPR_WASM_SOURCE_DIR}/test/run-tests.py)
add_custom_target(test
  COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY}
              -e ${SEXPR_WASM_BINARY_DIR}/sexpr-wasm
  DEPENDS sexpr-wasm
  WORKING_DIRECTORY ${SEXPR_WASM_SOURCE_DIR}
)

if (COMPILER_IS_CLANG)
  macro(add_sanitizer EXECUTABLE FLAGS TEST_TARGET)
    add_executable(${EXECUTABLE} EXCLUDE_FROM_ALL ${SEXPR_WASM_SRCS})
    add_dependencies(everything ${EXECUTABLE})
    set_target_properties(${EXECUTABLE}
      PROPERTIES
      COMPILE_FLAGS ${FLAGS}
      LINK_FLAGS ${FLAGS}
    )
    add_custom_target(${TEST_TARGET}
      COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY}
                  -e ${SEXPR_WASM_BINARY_DIR}/${EXECUTABLE}
      DEPENDS ${EXECUTABLE}
      WORKING_DIRECTORY ${SEXPR_WASM_SOURCE_DIR}
    )
  endmacro ()

  add_sanitizer(sexpr-wasm-asan -fsanitize=address test-asan)
  add_sanitizer(sexpr-wasm-msan -fsanitize=memory test-msan)
  add_sanitizer(sexpr-wasm-lsan -fsanitize=leak test-lsan)
endif ()

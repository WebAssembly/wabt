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

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckTypeSize)

check_include_file("alloca.h" HAVE_ALLOCA_H)
check_include_file("getopt.h" HAVE_GETOPT_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(sysconf "unistd.h" HAVE_SYSCONF)
check_type_size(ssize_t SSIZE_T)
check_type_size(int SIZEOF_INT BUILTIN_TYPES_ONLY)
check_type_size(long SIZEOF_LONG BUILTIN_TYPES_ONLY)
check_type_size("long long" SIZEOF_LONG_LONG BUILTIN_TYPES_ONLY)

configure_file(
  ${SEXPR_WASM_SOURCE_DIR}/src/wasm-config.h.in
  ${SEXPR_WASM_BINARY_DIR}/wasm-config.h
)

include_directories(src ${SEXPR_WASM_BINARY_DIR})

if (COMPILER_IS_MSVC)
  # disable warning C4018: signed/unsigned mismatch
  # disable warning C4056, C4756: overflow in floating-point constant arithmetic
  #   seems to not like float compare w/ HUGE_VALF; bug?
  add_definitions(-W3 -WX -wd4018 -wd4056 -wd4756 -D_CRT_SECURE_NO_WARNINGS)
else ()
  add_definitions(-Wall -Werror -Wpointer-arith -g)
endif ()

option(RUN_FLEX_BISON "run flex and bison" ON)
find_package(FLEX)
find_package(BISON 3.0)
if (RUN_FLEX_BISON AND FLEX_FOUND AND BISON_FOUND)
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

if (NOT COMPILER_IS_MSVC)
  set_source_files_properties(
    ${FLEX_LEXER_C} PROPERTIES COMPILE_FLAGS -Wno-unused-function
  )
endif ()

# sexpr-wasm
set(SEXPR_WASM_SRCS
  src/sexpr-wasm.c
  src/wasm.c
  src/wasm-allocator.c
  src/wasm-ast.c
  src/wasm-binary-writer.c
  src/wasm-check.c
  src/wasm-parse-literal.c
  src/wasm-stack-allocator.c
  src/wasm-vector.c
  src/wasm-writer.c
  ${BISON_PARSER_C}
  ${FLEX_LEXER_C}
)
add_executable(sexpr-wasm ${SEXPR_WASM_SRCS})

# hexfloat-test
find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
  set(HEXFLOAT_TEST_SRCS
    src/wasm-parse-literal.c
    test/hexfloat.c
  )
  add_executable(hexfloat_test ${HEXFLOAT_TEST_SRCS})
  target_link_libraries(hexfloat_test ${CMAKE_THREAD_LIBS_INIT})
endif ()

add_custom_target(everything DEPENDS sexpr-wasm hexfloat_test)

# test running
find_package(PythonInterp 2.7 REQUIRED)
set(RUN_TESTS_PY ${SEXPR_WASM_SOURCE_DIR}/test/run-tests.py)
add_custom_target(test
  COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY} -e $<TARGET_FILE:sexpr-wasm>
  DEPENDS sexpr-wasm
  WORKING_DIRECTORY ${SEXPR_WASM_SOURCE_DIR}
)

# sanitizer builds
if (COMPILER_IS_CLANG)
  macro(add_sanitizer_exe EXE_NAME FLAGS SRCS)
    add_executable(${EXE_NAME} EXCLUDE_FROM_ALL ${SRCS})
    add_dependencies(everything ${EXE_NAME})
    set_target_properties(${EXE_NAME}
      PROPERTIES
      COMPILE_FLAGS ${FLAGS}
      LINK_FLAGS ${FLAGS}
    )
  endmacro ()

  macro(add_sanitizer SANITIZER_NAME FLAGS)
    add_sanitizer_exe(sexpr-wasm-${SANITIZER_NAME} ${FLAGS} "${SEXPR_WASM_SRCS}")
    add_sanitizer_exe(hexfloat_test-${SANITIZER_NAME} ${FLAGS} "${HEXFLOAT_TEST_SRCS}")

    add_custom_target(all-${SANITIZER_NAME}
      DEPENDS sexpr-wasm-${SANITIZER_NAME} hexfloat_test-${SANITIZER_NAME})

    add_custom_target(test-${SANITIZER_NAME}
      COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY}
          -e $<TARGET_FILE:sexpr-wasm-${SANITIZER_NAME}>
      DEPENDS sexpr-wasm-${SANITIZER_NAME}
      WORKING_DIRECTORY ${SEXPR_WASM_SOURCE_DIR}
    )
  endmacro ()

  add_sanitizer(asan -fsanitize=address)
  add_sanitizer(msan -fsanitize=memory)
  add_sanitizer(lsan -fsanitize=leak)
endif ()

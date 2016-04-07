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
check_include_file("unistd.h" HAVE_UNISTD_H)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(sysconf "unistd.h" HAVE_SYSCONF)
check_type_size(ssize_t SSIZE_T)
check_type_size(size_t SIZEOF_SIZE_T)
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
  # disable -Wunused-parameter: this is really common when implementing
  #   interfaces, etc.
  # disable -Wpointer-arith: this is a GCC extension, and doesn't work in MSVC.
  add_definitions(
    -Wall -Wextra -Werror -Wno-unused-parameter -Wpointer-arith -g
  )

  if (COMPILER_IS_GNU)
    # disable -Wclobbered: it seems to be guessing incorrectly about a local
    # variable being clobbered by longjmp.
    add_definitions(-Wno-clobbered)
  endif ()

  # try to get the target architecture by compiling a dummy.c file and checking
  # the architecture using the file command.
  file(WRITE ${SEXPR_WASM_BINARY_DIR}/dummy.c "main(){}")
  try_compile(
    COMPILE_OK
    ${SEXPR_WASM_BINARY_DIR}
    ${SEXPR_WASM_BINARY_DIR}/dummy.c
    COPY_FILE ${SEXPR_WASM_BINARY_DIR}/dummy
  )
  if (COMPILE_OK)
    execute_process(
      COMMAND file ${SEXPR_WASM_BINARY_DIR}/dummy
      RESULT_VARIABLE FILE_RESULT
      OUTPUT_VARIABLE FILE_OUTPUT
      ERROR_QUIET
    )

    if (FILE_RESULT EQUAL 0)
      if (${FILE_OUTPUT} MATCHES "x86-64")
        set(TARGET_ARCH "x86-64")
      elseif (${FILE_OUTPUT} MATCHES "Intel 80386")
        set(TARGET_ARCH "i386")
      elseif (${FILE_OUTPUT} MATCHES "ARM")
        set(TARGET_ARCH "ARM")
      else ()
        message(WARNING "Unknown target architecture!")
      endif ()
    else ()
      message(WARNING "Error running file on dummy executable")
    endif ()
  else ()
    message(WARNING "Error compiling dummy.c file")
  endif ()

  if (TARGET_ARCH STREQUAL "i386")
    # wasm doesn't allow for x87 floating point math
    add_definitions(-msse2 -mfpmath=sse)
  endif ()
endif ()

option(RUN_FLEX_BISON "run flex and bison" ON)
find_package(FLEX)
find_package(BISON 3.0)
if (RUN_FLEX_BISON AND FLEX_FOUND AND BISON_FOUND)
  set(BISON_PARSER_C ${SEXPR_WASM_BINARY_DIR}/wasm-bison-parser.c)
  BISON_TARGET(WASM_BISON_PARSER
    ${SEXPR_WASM_SOURCE_DIR}/src/wasm-bison-parser.y
    ${BISON_PARSER_C}
    COMPILE_FLAGS --defines=${SEXPR_WASM_BINARY_DIR}/wasm-bison-parser.h
  )

  set(FLEX_LEXER_C ${SEXPR_WASM_BINARY_DIR}/wasm-flex-lexer.c)
  FLEX_TARGET(WASM_FLEX_LEXER
    ${SEXPR_WASM_SOURCE_DIR}/src/wasm-flex-lexer.l
    ${FLEX_LEXER_C}
    COMPILE_FLAGS -Cf
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

add_custom_target(everything)

add_library(wasm-common STATIC
  src/wasm-allocator.c
  src/wasm-common.c
  src/wasm-config.c
  src/wasm-option-parser.c
  src/wasm-stack-allocator.c
  src/wasm-vector.c
  src/wasm-writer.c
)

# sexpr-wasm
add_executable(sexpr-wasm
  src/sexpr-wasm.c
  src/wasm-ast.c
  src/wasm-ast-checker.c
  src/wasm-binary-writer.c
  src/wasm-binary-writer-spec.c
  src/wasm-literal.c
  src/wasm-mark-used-blocks.c
  src/wasm-parser-lexer-shared.c
  ${BISON_PARSER_C}
  ${FLEX_LEXER_C}
)
add_dependencies(everything sexpr-wasm)
target_link_libraries(sexpr-wasm wasm-common)

# wasm-wast
add_executable(wasm-wast
  src/wasm-ast.c
  src/wasm-ast-writer.c
  src/wasm-binary-reader.c
  src/wasm-binary-reader-ast.c
  src/wasm-literal.c
  src/wasm-wast.c
)
add_dependencies(everything wasm-wast)
target_link_libraries(wasm-wast wasm-common)

# wasm-interp-common
add_library(wasm-interp-common
  src/wasm-binary-reader.c
  src/wasm-binary-reader-interpreter.c
  src/wasm-interpreter.c
)

# wasm-interp
add_executable(wasm-interp src/wasm-interp.c)
add_dependencies(everything wasm-interp)
target_link_libraries(wasm-interp wasm-interp-common wasm-common m)

# stuff that requires squirrel
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/squirrel/squirrel)
  add_library(libsquirrel STATIC
    third_party/squirrel/squirrel/sqapi.cpp
    third_party/squirrel/squirrel/sqcompiler.cpp
    third_party/squirrel/squirrel/sqlexer.cpp
    third_party/squirrel/squirrel/sqstate.cpp
    third_party/squirrel/squirrel/sqbaselib.cpp
    third_party/squirrel/squirrel/sqdebug.cpp
    third_party/squirrel/squirrel/sqmem.cpp
    third_party/squirrel/squirrel/sqtable.cpp
    third_party/squirrel/squirrel/sqclass.cpp
    third_party/squirrel/squirrel/sqfuncstate.cpp
    third_party/squirrel/squirrel/sqobject.cpp
    third_party/squirrel/squirrel/sqvm.cpp
    third_party/squirrel/sqstdlib/sqstdaux.cpp
    third_party/squirrel/sqstdlib/sqstdio.cpp
    third_party/squirrel/sqstdlib/sqstdrex.cpp
    third_party/squirrel/sqstdlib/sqstdstring.cpp
    third_party/squirrel/sqstdlib/sqstdblob.cpp
    third_party/squirrel/sqstdlib/sqstdmath.cpp
    third_party/squirrel/sqstdlib/sqstdstream.cpp
    third_party/squirrel/sqstdlib/sqstdsystem.cpp
  )
  set_target_properties(libsquirrel PROPERTIES OUTPUT_NAME squirrel)
  include_directories(third_party/squirrel/include)

  if (COMPILER_IS_GNU)
    set_target_properties(libsquirrel
      PROPERTIES
      COMPILE_FLAGS -Wno-strict-aliasing
    )
  endif ()

  # standalone squirrel interpreter
  add_executable(sq EXCLUDE_FROM_ALL third_party/squirrel/sq/sq.c)
  target_link_libraries(sq libsquirrel)

  # wasm-interp-sq
  add_executable(wasm-interp-sq src/wasm-interp-sq.c)
  add_dependencies(everything wasm-interp-sq)
  target_link_libraries(wasm-interp-sq
    wasm-interp-common
    wasm-common
    libsquirrel
    m
  )
endif ()


# hexfloat-test
option(BUILD_TESTS "Build GTest-based tests" ON)
find_package(Threads)
if (BUILD_TESTS AND CMAKE_USE_PTHREADS_INIT)
  if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/googletest)
    message(FATAL_ERROR "Can't find third_party/gtest. Run git submodule update --init, or disable with CMake -DBUILD_TESTS=OFF.")
  endif ()

  set(HEXFLOAT_TEST_SRCS
    src/wasm-literal.c
    test/hexfloat.cc
    third_party/gtest/googletest/src/gtest-all.cc
    third_party/gtest/googletest/src/gtest_main.cc
  )
  include_directories(
    third_party/gtest/googletest
    third_party/gtest/googletest/include
  )
  add_executable(hexfloat_test ${HEXFLOAT_TEST_SRCS})
  add_dependencies(everything hexfloat_test)
  set_source_files_properties(hexfloat_test
    test/hexfloat.cc
    PROPERTIES
    COMPILE_FLAGS -std=c++11
  )
  target_link_libraries(hexfloat_test ${CMAKE_THREAD_LIBS_INIT})
endif ()

# test running
find_package(PythonInterp 2.7 REQUIRED)
set(RUN_TESTS_PY ${SEXPR_WASM_SOURCE_DIR}/test/run-tests.py)
add_custom_target(run-tests
  COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY}
      -e $<TARGET_FILE:sexpr-wasm>
      --wasm-wast $<TARGET_FILE:wasm-wast>
      --wasm-interp $<TARGET_FILE:wasm-interp>
  DEPENDS sexpr-wasm wasm-wast wasm-interp
  WORKING_DIRECTORY ${SEXPR_WASM_SOURCE_DIR}
)

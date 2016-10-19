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

option(BUILD_TESTS "Build GTest-based tests" ON)
option(RUN_BISON "Run bison" ON)
option(RUN_RE2C "Run re2c" ON)
option(USE_ASAN "Use address sanitizer" OFF)
option(USE_MSAN "Use memory sanitizer" OFF)
option(USE_LSAN "Use leak sanitizer" OFF)
option(USE_UBSAN "Use undefined behavior sanitizer" OFF)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  set(COMPILER_IS_CLANG 1)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 1)
  set(COMPILER_IS_MSVC 0)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 1)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
  set(COMPILER_IS_CLANG 1)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
else ()
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
endif ()

include(CheckIncludeFile)
include(CheckSymbolExists)

check_include_file("alloca.h" HAVE_ALLOCA_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(sysconf "unistd.h" HAVE_SYSCONF)

if (EMSCRIPTEN)
  set(SIZEOF_SSIZE_T 4)
  set(SIZEOF_SIZE_T 4)
  set(SIZEOF_INT 4)
  set(SIZEOF_LONG 4)
  set(SIZEOF_LONG_LONG 8)
else ()
  include(CheckTypeSize)
  check_type_size(ssize_t SSIZE_T)
  check_type_size(size_t SIZEOF_SIZE_T)
  check_type_size(int SIZEOF_INT BUILTIN_TYPES_ONLY)
  check_type_size(long SIZEOF_LONG BUILTIN_TYPES_ONLY)
  check_type_size("long long" SIZEOF_LONG_LONG BUILTIN_TYPES_ONLY)
endif ()

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

  if (NOT EMSCRIPTEN)
    # try to get the target architecture by compiling a dummy.c file and
    # checking the architecture using the file command.
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
        if (${FILE_OUTPUT} MATCHES "x86[-_]64")
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
endif ()

set(USE_SANITIZER FALSE)
function(SANITIZER NAME FLAGS)
  if (${NAME})
    message("HERE ${NAME} ${FLAGS}")
    if (USE_SANITIZER)
      message(FATAL_ERROR "Only one sanitizer allowed")
    endif()
    set(USE_SANITIZER TRUE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}" PARENT_SCOPE)
  endif ()
endfunction()
SANITIZER(USE_ASAN "-fsanitize=address")
SANITIZER(USE_MSAN "-fsanitize=memory")
SANITIZER(USE_LSAN "-fsanitize=leak")
SANITIZER(USE_UBSAN "-fsanitize=undefined -fno-sanitize-recover")

find_package(BISON 3.0)
if (RUN_BISON AND BISON_FOUND)
  set(WASM_AST_PARSER_GEN_C ${SEXPR_WASM_BINARY_DIR}/wasm-ast-parser-gen.c)
  BISON_TARGET(WASM_AST_PARSER_GEN_C
    ${SEXPR_WASM_SOURCE_DIR}/src/wasm-ast-parser.y
    ${WASM_AST_PARSER_GEN_C}
    COMPILE_FLAGS --defines=${SEXPR_WASM_BINARY_DIR}/wasm-ast-parser-gen.h
  )
else ()
  set(WASM_AST_PARSER_GEN_C src/prebuilt/wasm-ast-parser-gen.c)
  include_directories(src/prebuilt)
endif ()

if (COMPILER_IS_CLANG OR COMPILER_IS_GNU)
  # yyerror passes a non-string-literal to a printf-like function, which is a
  # warning.
  set_source_files_properties(
    ${WASM_AST_PARSER_GEN_C}
    PROPERTIES
    COMPILE_FLAGS -Wno-format-security
  )
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SEXPR_WASM_SOURCE_DIR}/cmake)
find_package(RE2C 0.13)
if (RUN_RE2C AND RE2C_EXECUTABLE)
  set(WASM_AST_LEXER_C ${SEXPR_WASM_SOURCE_DIR}/src/wasm-ast-lexer.c)
  set(WASM_AST_LEXER_GEN_C ${SEXPR_WASM_BINARY_DIR}/wasm-ast-lexer-gen.c)
  RE2C_TARGET(
    NAME WASM_AST_LEXER_GEN_C
    INPUT ${WASM_AST_LEXER_C}
    OUTPUT ${WASM_AST_LEXER_GEN_C}
    OPTIONS -bc
  )
else ()
  set(WASM_AST_LEXER_GEN_C src/prebuilt/wasm-ast-lexer-gen.c)
endif ()

add_custom_target(everything)

add_library(libwasm STATIC
  src/wasm-ast.c
  src/wasm-ast-parser-lexer-shared.c
  ${WASM_AST_LEXER_GEN_C}
  ${WASM_AST_PARSER_GEN_C}
  src/wasm-ast-checker.c
  src/wasm-binary-reader.c
  src/wasm-binary-writer.c
  src/wasm-binary-writer-spec.c
  src/wasm-binary-reader-ast.c
  src/wasm-ast-writer.c
  src/wasm-binary-reader-interpreter.c
  src/wasm-interpreter.c
  src/wasm-apply-names.c
  src/wasm-generate-names.c

  src/wasm-allocator.c
  src/wasm-common.c
  src/wasm-config.c
  src/wasm-literal.c
  src/wasm-option-parser.c
  src/wasm-stack-allocator.c
  src/wasm-stream.c
  src/wasm-vector.c
  src/wasm-writer.c
)
set_target_properties(libwasm PROPERTIES OUTPUT_NAME wasm)

if (NOT EMSCRIPTEN)
  # wast2wasm
  add_executable(wast2wasm src/exe/wast2wasm.c)
  add_dependencies(everything wast2wasm)
  target_link_libraries(wast2wasm libwasm)

  # wasm2wast
  add_executable(wasm2wast src/exe/wasm2wast.c)
  add_dependencies(everything wasm2wast)
  target_link_libraries(wasm2wast libwasm)

  # wasmopcodecnt
  add_executable(wasmopcodecnt src/exe/wasmopcodecnt.c
    src/wasm-binary-reader-opcnt.c)
  add_dependencies(everything wasmopcodecnt)
  target_link_libraries(wasmopcodecnt libwasm)

  # wasmdump
  add_executable(wasmdump src/exe/wasmdump.c src/wasm-binary-reader-objdump.c)
  add_dependencies(everything wasmdump)
  target_link_libraries(wasmdump libwasm)

  # wasm-interp
  add_executable(wasm-interp src/exe/wasm-interp.c)
  add_dependencies(everything wasm-interp)
  target_link_libraries(wasm-interp libwasm)
  if (COMPILER_IS_CLANG OR COMPILER_IS_GNU)
    target_link_libraries(wasm-interp m)
  endif ()

  # wast-desugar
  add_executable(wast-desugar src/exe/wast-desugar.c)
  add_dependencies(everything wast-desugar)
  target_link_libraries(wast-desugar libwasm)

  # symlinks for wast2wasm and wasm2wast
  if (NOT WIN32)
    add_custom_command(
      TARGET wast2wasm
      POST_BUILD
      COMMAND ln -sf wast2wasm sexpr-wasm
    )
    add_custom_command(
      TARGET wasm2wast
      POST_BUILD
      COMMAND ln -sf wasm2wast wasm-wast
    )
  endif ()
  set_property(
    DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "wasm2wast" "wast2wasm")

  # hexfloat-test
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
    set_source_files_properties(
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
        --wast2wasm $<TARGET_FILE:wast2wasm>
        --wasm2wast $<TARGET_FILE:wasm2wast>
        --wasm-interp $<TARGET_FILE:wasm-interp>
    DEPENDS wast2wasm wasm2wast wasm-interp
    WORKING_DIRECTORY ${SEXPR_WASM_SOURCE_DIR}
  )

  # install
  install(
    TARGETS wast2wasm wasm2wast wasm-interp wasmopcodecnt wasmdump wast-desugar
    DESTINATION bin
  )

else ()
  # emscripten stuff

  # just dump everything into one binary so we can reference it from JavaScript
  add_definitions(-Wno-warn-absolute-paths)
  add_executable(libwasmjs src/wasm-emscripten-helpers.c)
  add_dependencies(everything libwasmjs)
  target_link_libraries(libwasmjs libwasm)
  set_target_properties(libwasmjs PROPERTIES OUTPUT_NAME libwasm)

  set(WASM_JS ${SEXPR_WASM_SOURCE_DIR}/src/wasm.js)
  set(EMSCRIPTEN_EXPORTED_JSON
    ${SEXPR_WASM_SOURCE_DIR}/src/emscripten-exported.json)

  set(LIBWASM_LINK_FLAGS
    --pre-js ${WASM_JS}
    -s EXPORTED_FUNCTIONS=\"@${EMSCRIPTEN_EXPORTED_JSON}\"
    -s RESERVED_FUNCTION_POINTERS=10
    -s NO_EXIT_RUNTIME=1
  )
  string(REPLACE ";" " " LIBWASM_LINK_FLAGS_STR "${LIBWASM_LINK_FLAGS}")

  set_target_properties(libwasmjs
    PROPERTIES
    LINK_FLAGS "${LIBWASM_LINK_FLAGS_STR}"
    LINK_DEPENDS "${WASM_JS};${EMSCRIPTEN_EXPORTED_JSON}"
  )
endif ()

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
project(WABT)

option(BUILD_TESTS "Build GTest-based tests" ON)
option(RUN_RE2C "Run re2c" ON)
option(USE_ASAN "Use address sanitizer" OFF)
option(USE_MSAN "Use memory sanitizer" OFF)
option(USE_LSAN "Use leak sanitizer" OFF)
option(USE_UBSAN "Use undefined behavior sanitizer" OFF)
option(CODE_COVERAGE "Build with code coverage enabled" OFF)
option(WITH_EXCEPTIONS "Build with exceptions enabled" OFF)

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
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
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
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)

if (WIN32)
  check_symbol_exists(ENABLE_VIRTUAL_TERMINAL_PROCESSING "windows.h" HAVE_WIN32_VT100)
endif ()

if (EMSCRIPTEN)
  set(SIZEOF_SSIZE_T 4)
  set(SIZEOF_SIZE_T 4)
else ()
  include(CheckTypeSize)
  check_type_size(ssize_t SSIZE_T)
  check_type_size(size_t SIZEOF_SIZE_T)
endif ()

configure_file(
  ${WABT_SOURCE_DIR}/src/config.h.in
  ${WABT_BINARY_DIR}/config.h
)

include_directories(${WABT_SOURCE_DIR} ${WABT_BINARY_DIR})

if (COMPILER_IS_MSVC)
  # disable warning C4018: signed/unsigned mismatch
  # disable warning C4056, C4756: overflow in floating-point constant arithmetic
  #   seems to not like float compare w/ HUGE_VALF; bug?
  # disable warnings C4267 and C4244: conversion/truncation from larger to smaller type.
  # disable warning C4800: implicit conversion from larger int to bool
  add_definitions(-W3 -wd4018 -wd4056 -wd4756 -wd4267 -wd4244 -wd4800 -WX -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

  if (NOT WITH_EXCEPTIONS)
    # disable exception use in C++ library
    add_definitions(-D_HAS_EXCEPTIONS=0)
  endif ()
else ()
  # disable -Wunused-parameter: this is really common when implementing
  #   interfaces, etc.
  # disable -Wpointer-arith: this is a GCC extension, and doesn't work in MSVC.
  add_definitions(
    -Wall -Wextra -Werror -Wno-unused-parameter -Wpointer-arith -g -std=c++11
    -Wold-style-cast -Wuninitialized
  )

  if (NOT WITH_EXCEPTIONS)
    add_definitions(-fno-exceptions)
  endif ()

  # Need to define __STDC_*_MACROS because C99 specifies that C++ shouldn't
  # define format (e.g. PRIu64) or limit (e.g. UINT32_MAX) macros without the
  # definition, and some libcs (e.g. glibc2.17 and earlier) follow that.
  add_definitions(-D__STDC_LIMIT_MACROS=1 -D__STDC_FORMAT_MACROS=1)

  if (MINGW)
    # _POSIX is needed to ensure we use mingw printf
    # instead of the VC runtime one.
    add_definitions(-D_POSIX)
  endif ()

  if (COMPILER_IS_GNU)
    # disable -Wclobbered: it seems to be guessing incorrectly about a local
    # variable being clobbered by longjmp.
    add_definitions(-Wno-clobbered)
  endif ()

  if (NOT EMSCRIPTEN)
    # try to get the target architecture by compiling a dummy.c file and
    # checking the architecture using the file command.
    file(WRITE ${WABT_BINARY_DIR}/dummy.c "main(){}")
    try_compile(
      COMPILE_OK
      ${WABT_BINARY_DIR}
      ${WABT_BINARY_DIR}/dummy.c
      COPY_FILE ${WABT_BINARY_DIR}/dummy
    )
    if (COMPILE_OK)
      execute_process(
        COMMAND file ${WABT_BINARY_DIR}/dummy
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
    if (USE_SANITIZER)
      message(FATAL_ERROR "Only one sanitizer allowed")
    endif ()
    set(USE_SANITIZER TRUE PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}" PARENT_SCOPE)
  endif ()
endfunction()
SANITIZER(USE_ASAN "-fsanitize=address")
SANITIZER(USE_MSAN "-fsanitize=memory")
SANITIZER(USE_LSAN "-fsanitize=leak")

if (USE_UBSAN)
  # -fno-sanitize-recover was deprecated, see if we are compiling with a newer
  # clang that requires -fno-sanitize-recover=all.
  set(UBSAN_BLACKLIST ${WABT_SOURCE_DIR}/ubsan.blacklist)
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-fsanitize=undefined -fno-sanitize-recover -Wall -Werror" HAS_UBSAN_RECOVER_BARE)
  if (HAS_UBSAN_RECOVER_BARE)
    SANITIZER(USE_UBSAN "-fsanitize=undefined -fno-sanitize-recover -fsanitize-blacklist=${UBSAN_BLACKLIST}")
  endif ()
  CHECK_CXX_COMPILER_FLAG("-fsanitize=undefined -fno-sanitize-recover=all -Wall -Werror" HAS_UBSAN_RECOVER_ALL)
  if (HAS_UBSAN_RECOVER_ALL)
    SANITIZER(USE_UBSAN "-fsanitize=undefined -fno-sanitize-recover=all -fsanitize-blacklist=${UBSAN_BLACKLIST}")
  endif ()
  if (NOT USE_SANITIZER)
    message(FATAL_ERROR "UBSAN is not supported")
  endif ()
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${WABT_SOURCE_DIR}/cmake)
find_package(RE2C)
if (RUN_RE2C AND RE2C_EXECUTABLE AND (NOT ${RE2C_VERSION} VERSION_LESS "0.16"))
  message(STATUS "Using generated re2c lexer")
  set(WAST_LEXER_CC ${WABT_SOURCE_DIR}/src/wast-lexer.cc)
  set(WAST_LEXER_GEN_CC ${WABT_BINARY_DIR}/wast-lexer-gen.cc)
  RE2C_TARGET(
    NAME WAST_LEXER_GEN_CC
    INPUT ${WAST_LEXER_CC}
    OUTPUT ${WAST_LEXER_GEN_CC}
    OPTIONS -bc8 -W -Werror
  )
else ()
  message(STATUS "Using prebuilt re2c lexer")
  set(WAST_LEXER_GEN_CC src/prebuilt/wast-lexer-gen.cc)
endif ()

add_custom_target(everything)

add_library(libwabt STATIC
  src/token.cc
  src/opcode.cc
  src/error-handler.cc
  src/hash-util.cc
  src/filenames.cc
  src/string-view.cc
  src/ir.cc
  src/expr-visitor.cc
  src/lexer-source.cc
  src/lexer-source-line-finder.cc
  src/wast-parser-lexer-shared.cc
  ${WAST_LEXER_GEN_CC}
  src/wast-parser.cc
  src/type-checker.cc
  src/validator.cc

  src/binary-reader.cc
  src/binary-reader-logging.cc
  src/binary-writer.cc
  src/binary-writer-spec.cc
  src/binary-reader-ir.cc
  src/binding-hash.cc
  src/wat-writer.cc
  src/interp.cc
  src/binary-reader-interp.cc
  src/apply-names.cc
  src/generate-names.cc
  src/resolve-names.cc

  src/binary.cc
  src/color.cc
  src/common.cc
  src/config.cc
  src/feature.cc
  src/leb128.cc
  src/literal.cc
  src/option-parser.cc
  src/stream.cc
  src/tracing.cc
  src/utf8.cc
)
set_target_properties(libwabt PROPERTIES OUTPUT_NAME wabt)

if (NOT EMSCRIPTEN)
  if (CODE_COVERAGE)
    add_definitions("-fprofile-arcs -ftest-coverage")
    if (COMPILER_IS_CLANG)
      set(CMAKE_EXE_LINKER_FLAGS "--coverage")
    else ()
      link_libraries(gcov)
    endif ()
  endif ()

  function(wabt_executable name)
    # ARGV contains all arguments; remove the first one, ${name}, so it's just
    # a list of sources.
    list(REMOVE_AT ARGV 0)

    add_executable(${name} ${ARGV})
    add_dependencies(everything ${name})
    target_link_libraries(${name} libwabt)
    set_property(TARGET ${name} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${name} PROPERTY CXX_STANDARD_REQUIRED ON)
    list(APPEND WABT_EXECUTABLES ${name})
    set(WABT_EXECUTABLES ${WABT_EXECUTABLES} PARENT_SCOPE)

    add_custom_target(${name}-copy-to-bin ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${WABT_SOURCE_DIR}/bin
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> ${WABT_SOURCE_DIR}/bin
      DEPENDS ${name}
    )
  endfunction()

  # wat2wasm
  wabt_executable(wat2wasm src/tools/wat2wasm.cc)

  # wast2json
  wabt_executable(wast2json src/tools/wast2json.cc)

  # wasm2wat
  wabt_executable(wasm2wat src/tools/wasm2wat.cc)

  # wasm-opcodecnt
  wabt_executable(wasm-opcodecnt
    src/tools/wasm-opcodecnt.cc src/binary-reader-opcnt.cc)

  # wasm-objdump
  wabt_executable(wasm-objdump
    src/tools/wasm-objdump.cc src/binary-reader-objdump.cc)

  # wasm-link
  wabt_executable(wasm-link src/tools/wasm-link.cc src/binary-reader-linker.cc)

  # wasm-interp
  wabt_executable(wasm-interp src/tools/wasm-interp.cc)
  if (COMPILER_IS_CLANG OR COMPILER_IS_GNU)
    target_link_libraries(wasm-interp m)
  endif ()

  # spectest-interp
  wabt_executable(spectest-interp src/tools/spectest-interp.cc)
  if (COMPILER_IS_CLANG OR COMPILER_IS_GNU)
    target_link_libraries(spectest-interp m)
  endif ()

  # wat-desugar
  wabt_executable(wat-desugar src/tools/wat-desugar.cc)

  # wasm-validate
  wabt_executable(wasm-validate src/tools/wasm-validate.cc)

  find_package(Threads)
  if (BUILD_TESTS)
    if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/googletest)
      message(FATAL_ERROR "Can't find third_party/gtest. Run git submodule update --init, or disable with CMake -DBUILD_TESTS=OFF.")
    endif ()

    include_directories(
      third_party/gtest/googletest
      third_party/gtest/googletest/include
    )

    # gtest
    add_library(libgtest STATIC
      third_party/gtest/googletest/src/gtest-all.cc
    )

    # hexfloat-test
    set(HEXFLOAT_TEST_SRCS
      src/literal.cc
      test/hexfloat.cc
      third_party/gtest/googletest/src/gtest_main.cc
    )
    add_executable(hexfloat_test ${HEXFLOAT_TEST_SRCS})
    add_dependencies(everything hexfloat_test)
    target_link_libraries(hexfloat_test libgtest ${CMAKE_THREAD_LIBS_INIT})
    set_property(TARGET hexfloat_test PROPERTY CXX_STANDARD 11)
    set_property(TARGET hexfloat_test PROPERTY CXX_STANDARD_REQUIRED ON)

    # wabt-unittests
    set(UNITTESTS_SRCS
      src/test-circular-array.cc
      src/test-intrusive-list.cc
      src/test-string-view.cc
      src/test-filenames.cc
      src/test-utf8.cc
      src/test-wast-parser.cc
      third_party/gtest/googletest/src/gtest_main.cc
    )
    wabt_executable(wabt-unittests ${UNITTESTS_SRCS})
    target_link_libraries(wabt-unittests libgtest ${CMAKE_THREAD_LIBS_INIT})
  endif ()

  if (NOT CMAKE_VERSION VERSION_LESS "3.2")
    set(USES_TERMINAL USES_TERMINAL)
  endif ()

  # test running
  find_package(PythonInterp 3.4 REQUIRED)
  set(RUN_TESTS_PY ${WABT_SOURCE_DIR}/test/run-tests.py)
  add_custom_target(run-tests
    COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY} --bindir ${CMAKE_BINARY_DIR}
    DEPENDS ${WABT_EXECUTABLES}
    WORKING_DIRECTORY ${WABT_SOURCE_DIR}
    ${USES_TERMINAL}
  )

  # install
  install(TARGETS ${WABT_EXECUTABLES} DESTINATION bin)

else ()
  # emscripten stuff

  # just dump everything into one binary so we can reference it from JavaScript
  add_definitions(-Wno-warn-absolute-paths)
  add_executable(libwabtjs src/emscripten-helpers.cc)
  add_dependencies(everything libwabtjs)
  target_link_libraries(libwabtjs libwabt)
  set_target_properties(libwabtjs PROPERTIES OUTPUT_NAME libwabt)

  set(WABT_PRE_JS ${WABT_SOURCE_DIR}/src/wabt.pre.js)
  set(WABT_POST_JS ${WABT_SOURCE_DIR}/src/wabt.post.js)
  set(EMSCRIPTEN_EXPORTED_JSON ${WABT_SOURCE_DIR}/src/emscripten-exported.json)

  set(LIBWABT_LINK_FLAGS
    --memory-init-file 0
    --pre-js ${WABT_PRE_JS}
    --post-js ${WABT_POST_JS}
    -s EXPORTED_FUNCTIONS=\"@${EMSCRIPTEN_EXPORTED_JSON}\"
    -s RESERVED_FUNCTION_POINTERS=10
    -s NO_EXIT_RUNTIME=1
  )
  string(REPLACE ";" " " LIBWABT_LINK_FLAGS_STR "${LIBWABT_LINK_FLAGS}")

  set_target_properties(libwabtjs
    PROPERTIES
    LINK_FLAGS "${LIBWABT_LINK_FLAGS_STR}"
    LINK_DEPENDS "${WABT_PRE_JS};${WABT_POST_JS};${EMSCRIPTEN_EXPORTED_JSON}"
  )
endif ()

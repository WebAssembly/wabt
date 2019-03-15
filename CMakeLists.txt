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
option(BUILD_TOOLS "Build wabt commandline tools" ON)
option(RUN_RE2C "Run re2c" ON)
option(USE_ASAN "Use address sanitizer" OFF)
option(USE_MSAN "Use memory sanitizer" OFF)
option(USE_LSAN "Use leak sanitizer" OFF)
option(USE_UBSAN "Use undefined behavior sanitizer" OFF)
option(CODE_COVERAGE "Build with code coverage enabled" OFF)
option(WITH_EXCEPTIONS "Build with exceptions enabled" OFF)
option(WERROR "Build with warnings as errors" OFF)

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

include(CheckTypeSize)
check_type_size(ssize_t SSIZE_T)
check_type_size(size_t SIZEOF_SIZE_T)

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
  add_definitions(-W3 -wd4018 -wd4056 -wd4756 -wd4267 -wd4244 -wd4800 -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

  if (WERROR)
    add_definitions(-WX)
  endif ()

  if (NOT WITH_EXCEPTIONS)
    # disable exception use in C++ library
    add_definitions(-D_HAS_EXCEPTIONS=0)
  endif ()
else ()
  # disable -Wunused-parameter: this is really common when implementing
  #   interfaces, etc.
  # disable -Wpointer-arith: this is a GCC extension, and doesn't work in MSVC.
  add_definitions(
    -Wall -Wextra -Wno-unused-parameter -Wpointer-arith -g
    -Wuninitialized
  )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wold-style-cast")

  if (WERROR)
    add_definitions(-Werror)
  endif ()

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

add_library(wabt STATIC
  src/apply-names.cc
  src/binary.cc
  src/binary-reader.cc
  src/binary-reader-ir.cc
  src/binary-reader-logging.cc
  src/binary-writer.cc
  src/binary-writer-spec.cc
  src/binding-hash.cc
  src/color.cc
  src/common.cc
  src/config.cc
  src/error-formatter.cc
  src/expr-visitor.cc
  src/feature.cc
  src/filenames.cc
  src/generate-names.cc
  src/hash-util.cc
  src/ir.cc
  src/leb128.cc
  src/lexer-source.cc
  src/lexer-source-line-finder.cc
  src/literal.cc
  src/opcode.cc
  src/opcode-code-table.c
  src/option-parser.cc
  src/resolve-names.cc
  src/stream.cc
  src/string-view.cc
  src/token.cc
  src/tracing.cc
  src/type-checker.cc
  src/utf8.cc
  src/validator.cc
  src/wast-parser.cc
  src/wat-writer.cc
  ${WAST_LEXER_GEN_CC}

  # TODO(binji): Move this into its own library?
  src/interp/binary-reader-interp.cc
  src/interp/interp.cc
  src/interp/interp-disassemble.cc
  src/interp/interp-trace.cc
)


if (NOT EMSCRIPTEN)
  if (CODE_COVERAGE)
    add_definitions("-fprofile-arcs -ftest-coverage")
    if (COMPILER_IS_CLANG)
      set(CMAKE_EXE_LINKER_FLAGS "--coverage")
    else ()
      link_libraries(gcov)
    endif ()
  endif ()

  include(CMakeParseArguments)
  function(wabt_executable)
    cmake_parse_arguments(EXE "WITH_LIBM;INSTALL" "NAME" "SOURCES;LIBS" ${ARGN})

    # Always link libwabt.
    set(EXE_LIBS "${EXE_LIBS};wabt")

    # Optionally link libm.
    if (EXE_WITH_LIBM AND (COMPILER_IS_CLANG OR COMPILER_IS_GNU))
      set(EXE_LIBS "${EXE_LIBS};m")
    endif ()

    add_executable(${EXE_NAME} ${EXE_SOURCES})
    add_dependencies(everything ${EXE_NAME})
    target_link_libraries(${EXE_NAME} ${EXE_LIBS})
    set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${EXE_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

    if (EXE_INSTALL)
      list(APPEND WABT_EXECUTABLES ${EXE_NAME})
      set(WABT_EXECUTABLES ${WABT_EXECUTABLES} PARENT_SCOPE)

      add_custom_target(${EXE_NAME}-copy-to-bin ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WABT_SOURCE_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXE_NAME}> ${WABT_SOURCE_DIR}/bin
        DEPENDS ${EXE_NAME}
      )
    endif ()
  endfunction()

  if (BUILD_TOOLS)
    # wat2wasm
    wabt_executable(
      NAME wat2wasm
      SOURCES src/tools/wat2wasm.cc
      INSTALL
    )

    # wast2json
    wabt_executable(
      NAME wast2json
      SOURCES src/tools/wast2json.cc
      INSTALL
    )

    # wasm2wat
    wabt_executable(
      NAME wasm2wat
      SOURCES src/tools/wasm2wat.cc
      INSTALL
    )

    # wasm2c
    wabt_executable(
      NAME wasm2c
      SOURCES src/tools/wasm2c.cc src/c-writer.cc
      INSTALL
    )

    # wasm-opcodecnt
    wabt_executable(
      NAME wasm-opcodecnt
      SOURCES src/tools/wasm-opcodecnt.cc src/binary-reader-opcnt.cc
      INSTALL
    )

    # wasm-objdump
    wabt_executable(
      NAME wasm-objdump
      SOURCES src/tools/wasm-objdump.cc src/binary-reader-objdump.cc
      INSTALL
    )

    # wasm-interp
    wabt_executable(
      NAME wasm-interp
      SOURCES src/tools/wasm-interp.cc
      WITH_LIBM
      INSTALL
    )

    # spectest-interp
    wabt_executable(
      NAME spectest-interp
      SOURCES src/tools/spectest-interp.cc
      WITH_LIBM
      INSTALL
    )

    # wat-desugar
    wabt_executable(
      NAME wat-desugar
      SOURCES src/tools/wat-desugar.cc
      INSTALL
    )

    # wasm-validate
    wabt_executable(
      NAME wasm-validate
      SOURCES src/tools/wasm-validate.cc
      INSTALL
    )

    # wasm-strip
    wabt_executable(
      NAME wasm-strip
      SOURCES src/tools/wasm-strip.cc
      INSTALL
    )
  endif ()

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
      src/test-hexfloat.cc
      third_party/gtest/googletest/src/gtest_main.cc
    )
    wabt_executable(
      NAME hexfloat_test
      SOURCES ${HEXFLOAT_TEST_SRCS}
      LIBS libgtest ${CMAKE_THREAD_LIBS_INIT}
    )

    # wabt-unittests
    set(UNITTESTS_SRCS
      src/test-binary-reader.cc
      src/test-circular-array.cc
      src/test-interp.cc
      src/test-intrusive-list.cc
      src/test-literal.cc
      src/test-string-view.cc
      src/test-filenames.cc
      src/test-utf8.cc
      src/test-wast-parser.cc
      third_party/gtest/googletest/src/gtest_main.cc
    )
    wabt_executable(
      NAME wabt-unittests
      SOURCES ${UNITTESTS_SRCS}
      LIBS libgtest ${CMAKE_THREAD_LIBS_INIT}
    )

    if (NOT CMAKE_VERSION VERSION_LESS "3.2")
      set(USES_TERMINAL USES_TERMINAL)
    endif ()

    # test running
    find_package(PythonInterp 2.7 REQUIRED)
    set(RUN_TESTS_PY ${WABT_SOURCE_DIR}/test/run-tests.py)
    add_custom_target(run-tests
      COMMAND ${CMAKE_BINARY_DIR}/wabt-unittests
      COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY} --bindir ${CMAKE_BINARY_DIR}
      DEPENDS ${WABT_EXECUTABLES}
      WORKING_DIRECTORY ${WABT_SOURCE_DIR}
      ${USES_TERMINAL}
    )
  endif ()

  # install
  if (BUILD_TOOLS OR BUILD_TESTS)
    install(TARGETS ${WABT_EXECUTABLES} DESTINATION bin)
    if (UNIX)
      if (NOT CMAKE_INSTALL_MANDIR)
        include(GNUInstallDirs)
      endif ()
      file(GLOB MAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/man/*.1")
      foreach(MAN_FILE ${MAN_FILES})
        install(FILES ${MAN_FILE}
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/)
      endforeach()
    endif ()
  endif ()

else ()
  # emscripten stuff

  # just dump everything into one binary so we can reference it from JavaScript
  add_definitions(-Wno-warn-absolute-paths)
  add_executable(libwabtjs src/emscripten-helpers.cc)
  add_dependencies(everything libwabtjs)
  target_link_libraries(libwabtjs wabt)
  set_target_properties(libwabtjs PROPERTIES OUTPUT_NAME libwabt)

  set(WABT_POST_JS ${WABT_SOURCE_DIR}/src/wabt.post.js)
  set(EMSCRIPTEN_EXPORTED_JSON ${WABT_SOURCE_DIR}/src/emscripten-exported.json)

  set(LIBWABT_LINK_FLAGS
    --memory-init-file 0
    --post-js ${WABT_POST_JS}
    -s EXPORTED_FUNCTIONS=\"@${EMSCRIPTEN_EXPORTED_JSON}\"
    -s RESERVED_FUNCTION_POINTERS=10
    -s NO_EXIT_RUNTIME=1
    -s ALLOW_MEMORY_GROWTH=1
    -s MODULARIZE=1
    -s EXPORT_NAME=\"'WabtModule'\"
    -s WASM=0
    -Oz
    --llvm-lto 1
  )
  string(REPLACE ";" " " LIBWABT_LINK_FLAGS_STR "${LIBWABT_LINK_FLAGS}")

  set_target_properties(libwabtjs
    PROPERTIES
    LINK_FLAGS "${LIBWABT_LINK_FLAGS_STR}"
    LINK_DEPENDS "${WABT_POST_JS};${EMSCRIPTEN_EXPORTED_JSON}"
  )
endif ()

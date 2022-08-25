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

cmake_minimum_required(VERSION 3.16)
project(WABT LANGUAGES C CXX VERSION 1.0.29)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check if wabt is being used directly or via add_subdirectory, FetchContent, etc
string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" PROJECT_IS_TOP_LEVEL)

# By default use the project version as the version string
set(WABT_VERSION_STRING "${PROJECT_VERSION}")

# For git users, attempt to generate a more useful version string
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  find_package(Git)
  if (Git_FOUND)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${WABT_SOURCE_DIR}" describe --tags
      RESULT_VARIABLE GIT_VERSION_RESULT
      ERROR_VARIABLE GIT_VERSION_ERROR
      ERROR_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE GIT_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (GIT_VERSION_RESULT EQUAL 0)
      # If we're actually on the tag for this version, don't add the commit description
      if (NOT GIT_VERSION STREQUAL "${WABT_VERSION_STRING}")
        string(APPEND WABT_VERSION_STRING " (git~${GIT_VERSION})")
      endif ()
    elseif (PROJECT_IS_TOP_LEVEL)
      message(NOTICE "git: ${GIT_VERSION_ERROR}\n  ** Did you forget to run `git fetch --tags`?")
    endif ()
  endif ()
endif ()

option(BUILD_TESTS "Build GTest-based tests" ON)
option(USE_SYSTEM_GTEST "Use system GTest, instead of building" OFF)
option(BUILD_TOOLS "Build wabt commandline tools" ON)
option(BUILD_FUZZ_TOOLS "Build tools that can repro fuzz bugs" OFF)
option(BUILD_LIBWASM "Build libwasm" ON)
option(USE_ASAN "Use address sanitizer" OFF)
option(USE_MSAN "Use memory sanitizer" OFF)
option(USE_LSAN "Use leak sanitizer" OFF)
option(USE_UBSAN "Use undefined behavior sanitizer" OFF)
option(CODE_COVERAGE "Build with code coverage enabled" OFF)
option(WITH_EXCEPTIONS "Build with exceptions enabled" OFF)
option(WERROR "Build with warnings as errors" OFF)
option(WABT_INSTALL_RULES "Include WABT's install() rules" "${PROJECT_IS_TOP_LEVEL}")
# WASI support is still a work in progress.
# Only a handful of syscalls are supported at this point.
option(WITH_WASI "Build WASI support via uvwasi" OFF)

if (MSVC)
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 1)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(COMPILER_IS_CLANG 1)
  set(COMPILER_IS_GNU 0)
  set(COMPILER_IS_MSVC 0)
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(COMPILER_IS_CLANG 0)
  set(COMPILER_IS_GNU 1)
  set(COMPILER_IS_MSVC 0)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
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
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)

if (WIN32)
  check_symbol_exists(ENABLE_VIRTUAL_TERMINAL_PROCESSING "windows.h" HAVE_WIN32_VT100)
endif ()

include(CheckTypeSize)
check_type_size(ssize_t SSIZE_T)
check_type_size(size_t SIZEOF_SIZE_T)

include(TestBigEndian)  # Note: deprecated in CMake 3.20
test_big_endian(WABT_BIG_ENDIAN)

configure_file(src/config.h.in include/wabt/config.h @ONLY)


if (COMPILER_IS_MSVC)
  if (WERROR)
    add_definitions(-WX)
  endif ()

  # disable warning C4018: signed/unsigned mismatch
  # disable warning C4056, C4756: overflow in floating-point constant arithmetic
  #   seems to not like float compare w/ HUGE_VALF; bug?
  # disable warnings C4267 and C4244: conversion/truncation from larger to smaller type.
  # disable warning C4800: implicit conversion from larger int to bool
  add_definitions(-W3 -wd4018 -wd4056 -wd4756 -wd4267 -wd4244 -wd4800 -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

  if (NOT WITH_EXCEPTIONS)
    # disable exception use in C++ library
    add_definitions(-D_HAS_EXCEPTIONS=0)
  endif ()

  # multi-core build.
  add_definitions("/MP")

else ()
  if (WERROR)
    add_definitions(-Werror)
  endif ()

  # disable -Wunused-parameter: this is really common when implementing
  #   interfaces, etc.
  # disable -Wpointer-arith: this is a GCC extension, and doesn't work in MSVC.
  add_definitions(
    -Wall -Wextra -Wno-unused-parameter -Wpointer-arith -Wuninitialized
  )

  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")

  if (NOT WITH_EXCEPTIONS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
  endif ()

  # Need to define __STDC_*_MACROS because C99 specifies that C++ shouldn't
  # define format (e.g. PRIu64) or limit (e.g. UINT32_MAX) macros without the
  # definition, and some libcs (e.g. glibc2.17 and earlier) follow that.
  add_definitions(-D__STDC_LIMIT_MACROS=1 -D__STDC_FORMAT_MACROS=1)

  if (MINGW OR CYGWIN)
    # On MINGW, _POSIX_C_SOURCE is needed to ensure we use mingw printf
    # instead of the VC runtime one.
    add_definitions(-D_POSIX_C_SOURCE=200809L)
  endif()

  if (COMPILER_IS_GNU)
    # disable -Wclobbered: it seems to be guessing incorrectly about a local
    # variable being clobbered by longjmp.
    add_definitions(-Wno-clobbered)
  endif ()

  # wasm doesn't allow for x87 floating point math
  if (NOT EMSCRIPTEN)
    check_symbol_exists(__i386__ "" TARGET_IS_X86_32)
    check_symbol_exists(__SSE2_MATH__ "" HAVE_SSE2_MATH)

    if (TARGET_IS_X86_32 AND NOT HAVE_SSE2_MATH)
      if (COMPILER_IS_GNU OR COMPILER_IS_CLANG)
        add_definitions(-msse2 -mfpmath=sse)
      else ()
        message(
          WARNING
          "Unknown compiler ${CMAKE_CXX_COMPILER_ID} appears to target x86_32 with x87 "
          "math. Please add flags to use SSE2 math and set TARGET_IS_X86_32 and "
          "HAVE_SSE2_MATH appropriately at the CMake command line."
        )
      endif ()
    endif ()
  endif ()
endif ()

set(USE_SANITIZER FALSE)

function(sanitizer NAME FLAGS)
  if (${NAME})
    if (USE_SANITIZER)
      message(FATAL_ERROR "Only one sanitizer allowed")
    endif ()
    set(USE_SANITIZER TRUE PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}" PARENT_SCOPE)
  endif ()
endfunction()
sanitizer(USE_ASAN "-fsanitize=address")
sanitizer(USE_MSAN "-fsanitize=memory")
sanitizer(USE_LSAN "-fsanitize=leak")

if (USE_UBSAN)
  # -fno-sanitize-recover was deprecated, see if we are compiling with a newer
  # clang that requires -fno-sanitize-recover=all.
  set(UBSAN_BLACKLIST ${WABT_SOURCE_DIR}/ubsan.blacklist)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-fsanitize=undefined -fno-sanitize-recover -Wall -Werror" HAS_UBSAN_RECOVER_BARE)
  if (HAS_UBSAN_RECOVER_BARE)
    sanitizer(USE_UBSAN "-fsanitize=undefined -fno-sanitize-recover -fsanitize-blacklist=${UBSAN_BLACKLIST}")
  endif ()
  check_cxx_compiler_flag("-fsanitize=undefined -fno-sanitize-recover=all -Wall -Werror" HAS_UBSAN_RECOVER_ALL)
  # If we already setup UBSAN recover bare, setting it up again here will be an error.
  if (NOT USE_SANITIZER AND HAS_UBSAN_RECOVER_ALL)
    sanitizer(USE_UBSAN "-fsanitize=undefined -fno-sanitize-recover=all -fsanitize-blacklist=${UBSAN_BLACKLIST}")
  endif ()
  if (NOT USE_SANITIZER)
    message(FATAL_ERROR "UBSAN is not supported")
  endif ()
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${WABT_SOURCE_DIR}/cmake)

add_custom_target(everything)

set(WABT_LIBRARY_CC
  src/apply-names.cc
  src/binary-reader-ir.cc
  src/binary-reader-logging.cc
  src/binary-reader.cc
  src/binary-writer-spec.cc
  src/binary-writer.cc
  src/binary.cc
  src/binding-hash.cc
  src/color.cc
  src/common.cc
  src/config.cc
  src/config.h.in
  src/decompiler.cc
  src/error-formatter.cc
  src/expr-visitor.cc
  src/feature.cc
  src/filenames.cc
  src/generate-names.cc
  src/ir-util.cc
  src/ir.cc
  src/leb128.cc
  src/lexer-source-line-finder.cc
  src/lexer-source.cc
  src/literal.cc
  src/opcode-code-table.c
  src/opcode-code-table.c
  src/opcode.cc
  src/option-parser.cc
  src/resolve-names.cc
  src/shared-validator.cc
  src/stream.cc
  src/token.cc
  src/tracing.cc
  src/type-checker.cc
  src/utf8.cc
  src/validator.cc
  src/wast-lexer.cc
  src/wast-parser.cc
  src/wat-writer.cc

  # TODO(binji): Move this into its own library?
  src/interp/binary-reader-interp.cc
  src/interp/interp.cc
  src/interp/interp-util.cc
  src/interp/istream.cc
)

set(WABT_LIBRARY_H
  ${WABT_BINARY_DIR}/include/wabt/config.h

  include/wabt/apply-names.h
  include/wabt/binary-reader-ir.h
  include/wabt/binary-reader-logging.h
  include/wabt/binary-reader.h
  include/wabt/binary-writer-spec.h
  include/wabt/binary-writer.h
  include/wabt/binary.h
  include/wabt/binding-hash.h
  include/wabt/color.h
  include/wabt/common.h
  include/wabt/decompiler-ast.h
  include/wabt/decompiler-ls.h
  include/wabt/decompiler-naming.h
  include/wabt/decompiler.h
  include/wabt/error-formatter.h
  include/wabt/expr-visitor.h
  include/wabt/feature.h
  include/wabt/filenames.h
  include/wabt/generate-names.h
  include/wabt/ir-util.h
  include/wabt/ir.h
  include/wabt/leb128.h
  include/wabt/lexer-source-line-finder.h
  include/wabt/lexer-source.h
  include/wabt/literal.h
  include/wabt/opcode-code-table.h
  include/wabt/opcode.h
  include/wabt/option-parser.h
  include/wabt/resolve-names.h
  include/wabt/shared-validator.h
  include/wabt/stream.h
  include/wabt/string-util.h
  include/wabt/token.h
  include/wabt/tracing.h
  include/wabt/type-checker.h
  include/wabt/type.h
  include/wabt/utf8.h
  include/wabt/validator.h
  include/wabt/wast-lexer.h
  include/wabt/wast-parser.h
  include/wabt/wat-writer.h

  # TODO(binji): Move this into its own library?
  include/wabt/interp/binary-reader-interp.h
  include/wabt/interp/interp-inl.h
  include/wabt/interp/interp-math.h
  include/wabt/interp/interp-util.h
  include/wabt/interp/interp.h
  include/wabt/interp/istream.h
)

set(WABT_LIBRARY_SRC ${WABT_LIBRARY_CC} ${WABT_LIBRARY_H})

add_library(wabt STATIC ${WABT_LIBRARY_SRC})
add_library(wabt::wabt ALIAS wabt)

target_compile_features(wabt PUBLIC cxx_std_17)
target_include_directories(
  wabt
  PUBLIC
  "$<BUILD_INTERFACE:${WABT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${WABT_BINARY_DIR}/include>"
)

if (WABT_INSTALL_RULES)
  install(
    TARGETS wabt EXPORT wabt-targets
    COMPONENT wabt-development
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )
  install(
    DIRECTORY "${WABT_SOURCE_DIR}/include/" "${WABT_BINARY_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT wabt-development
  )
endif ()

IF (NOT WIN32)
  add_library(wasm-rt-impl STATIC wasm2c/wasm-rt-impl.c wasm2c/wasm-rt-impl.h)
  add_library(wabt::wasm-rt-impl ALIAS wasm-rt-impl)
  if (WABT_BIG_ENDIAN)
    target_compile_definitions(wasm-rt-impl PUBLIC WABT_BIG_ENDIAN=1)
  endif ()

  if (WABT_INSTALL_RULES)
    install(
      TARGETS wasm-rt-impl
      EXPORT wabt-targets
      COMPONENT wabt-development
      INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    install(
      FILES "wasm2c/wasm-rt-impl.h" "wasm2c/wasm-rt.h"
      TYPE INCLUDE
      COMPONENT wabt-development
    )
  endif ()
endif ()

if (BUILD_FUZZ_TOOLS)
  set(FUZZ_FLAGS "-fsanitize=fuzzer,address")
  add_library(wabt-fuzz STATIC ${WABT_LIBRARY_SRC})
  set_target_properties(wabt-fuzz
    PROPERTIES
    COMPILE_FLAGS "${FUZZ_FLAGS}"
  )
endif ()

# libwasm, which implenents the wasm C API
if (BUILD_LIBWASM)
  add_library(wasm SHARED ${WABT_LIBRARY_SRC} src/interp/interp-wasm-c-api.cc)
  target_link_libraries(wasm PUBLIC wabt)
  target_include_directories(wasm PUBLIC third_party/wasm-c-api/include)
  if (COMPILER_IS_MSVC)
    target_compile_definitions(wasm PRIVATE "WASM_API_EXTERN=__declspec(dllexport)")
  else ()
    target_compile_options(wasm PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-old-style-cast>)
    target_compile_definitions(wasm PRIVATE "WASM_API_EXTERN=__attribute__((visibility(\"default\")))")
  endif ()
  set_target_properties(wasm PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif ()

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
  cmake_parse_arguments(EXE "WITH_LIBM;FUZZ;INSTALL" "NAME" "SOURCES;LIBS" ${ARGN})

  # Always link libwabt.
  if (EXE_FUZZ)
    set(EXE_LIBS "${EXE_LIBS};wabt-fuzz")
    set(EXTRA_LINK_FLAGS "${FUZZ_FLAGS}")
  else ()
    set(EXE_LIBS "${EXE_LIBS};wabt")
  endif ()

  # Optionally link libm.
  if (EXE_WITH_LIBM AND (COMPILER_IS_CLANG OR COMPILER_IS_GNU))
    set(EXE_LIBS "${EXE_LIBS};m")
  endif ()

  add_executable(${EXE_NAME} ${EXE_SOURCES})
  add_dependencies(everything ${EXE_NAME})
  target_link_libraries(${EXE_NAME} ${EXE_LIBS})

  if (EMSCRIPTEN)
    # build to JS for now, as node.js doesn't have code caching for wasm yet,
    # and wasm startup times are slower
    set(EXTRA_LINK_FLAGS
      "${EXTRA_LINK_FLAGS} -s NODERAWFS -s SINGLE_FILE -s WASM=0 -Oz -s ALLOW_MEMORY_GROWTH=1"
    )
  endif ()

  set_target_properties(${EXE_NAME}
    PROPERTIES
    LINK_FLAGS "${EXTRA_LINK_FLAGS}"
  )

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
  set(TEMPLATE_CMAKE ${WABT_SOURCE_DIR}/scripts/gen-wasm2c-templates.cmake)

  # wasm2c generated code templates
  add_custom_command(
    OUTPUT wasm2c_header_top.cc wasm2c_header_bottom.cc wasm2c_source_includes.cc wasm2c_source_declarations.cc

    COMMAND ${CMAKE_COMMAND} -D out="wasm2c_header_top.cc" -D in="${WABT_SOURCE_DIR}/src/template/wasm2c.top.h" -D symbol="s_header_top" -P ${TEMPLATE_CMAKE}
    COMMAND ${CMAKE_COMMAND} -D out="wasm2c_header_bottom.cc" -D in="${WABT_SOURCE_DIR}/src/template/wasm2c.bottom.h" -D symbol="s_header_bottom" -P ${TEMPLATE_CMAKE}
    COMMAND ${CMAKE_COMMAND} -D out="wasm2c_source_includes.cc" -D in="${WABT_SOURCE_DIR}/src/template/wasm2c.includes.c" -D symbol="s_source_includes" -P ${TEMPLATE_CMAKE}
    COMMAND ${CMAKE_COMMAND} -D out="wasm2c_source_declarations.cc" -D in="${WABT_SOURCE_DIR}/src/template/wasm2c.declarations.c" -D symbol="s_source_declarations" -P ${TEMPLATE_CMAKE}

    DEPENDS ${WABT_SOURCE_DIR}/src/template/wasm2c.top.h
    ${WABT_SOURCE_DIR}/src/template/wasm2c.bottom.h
    ${WABT_SOURCE_DIR}/src/template/wasm2c.includes.c
    ${WABT_SOURCE_DIR}/src/template/wasm2c.declarations.c
    )

  add_library(cwriter-template wasm2c_header_top.cc wasm2c_header_bottom.cc wasm2c_source_includes.cc wasm2c_source_declarations.cc)

  wabt_executable(
    NAME wasm2c
    SOURCES src/tools/wasm2c.cc src/c-writer.cc
    LIBS cwriter-template
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

  if(WITH_WASI)
    add_subdirectory("third_party/uvwasi" EXCLUDE_FROM_ALL)
    include_directories(third_party/uvwasi/include)
    add_definitions(-DWITH_WASI)
    set(INTERP_LIBS uvwasi_a)
    set(EXTRA_INTERP_SRC src/interp/interp-wasi.cc)
  endif()

  # wasm-interp

  wabt_executable(
    NAME wasm-interp
    SOURCES src/tools/wasm-interp.cc ${EXTRA_INTERP_SRC}
    LIBS ${INTERP_LIBS}
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

  # wasm-decompile
  wabt_executable(
    NAME wasm-decompile
    SOURCES src/tools/wasm-decompile.cc
    INSTALL
  )

  if(BUILD_FUZZ_TOOLS)
    # wasm2wat-fuzz
    wabt_executable(
      NAME wasm2wat-fuzz
      SOURCES src/tools/wasm2wat-fuzz.cc
      FUZZ
      INSTALL
    )
  endif ()
endif ()

if (BUILD_TESTS)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

  # Python 3.5 is the version shipped in Ubuntu Xenial
  find_package(PythonInterp 3.5 REQUIRED)

  if (NOT USE_SYSTEM_GTEST)
    if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/googletest)
      message(FATAL_ERROR "Can't find third_party/gtest. Run git submodule update --init, or disable with CMake -DBUILD_TESTS=OFF.")
    endif ()

    include_directories(
      third_party/gtest/googletest
      third_party/gtest/googletest/include
    )

    # gtest
    add_library(gtest STATIC
      third_party/gtest/googletest/src/gtest-all.cc
    )

    add_library(gtest_main STATIC
      third_party/gtest/googletest/src/gtest_main.cc
    )
  endif ()

  # hexfloat-test
  set(HEXFLOAT_TEST_SRCS
    src/literal.cc
    src/test-hexfloat.cc
  )
  wabt_executable(
    NAME hexfloat_test
    SOURCES ${HEXFLOAT_TEST_SRCS}
    LIBS gtest_main gtest ${CMAKE_THREAD_LIBS_INIT}
  )

  # wabt-unittests
  set(UNITTESTS_SRCS
    src/test-binary-reader.cc
    src/test-circular-array.cc
    src/test-interp.cc
    src/test-intrusive-list.cc
    src/test-literal.cc
    src/test-option-parser.cc
    src/test-filenames.cc
    src/test-utf8.cc
    src/test-wast-parser.cc
  )
  wabt_executable(
    NAME wabt-unittests
    SOURCES ${UNITTESTS_SRCS}
    LIBS gtest_main gtest ${CMAKE_THREAD_LIBS_INIT}
  )

  # test running
  set(RUN_TESTS_PY ${WABT_SOURCE_DIR}/test/run-tests.py)

  add_custom_target(run-tests
    COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY} --bindir $<TARGET_FILE_DIR:wat2wasm>
    DEPENDS ${WABT_EXECUTABLES}
    WORKING_DIRECTORY ${WABT_SOURCE_DIR}
    USES_TERMINAL
  )

  add_custom_target(run-unittests
    COMMAND $<TARGET_FILE:wabt-unittests>
    DEPENDS wabt-unittests
    WORKING_DIRECTORY ${WABT_SOURCE_DIR}
    USES_TERMINAL
  )

  add_custom_target(run-c-api-tests
    COMMAND ${PYTHON_EXECUTABLE} ${WABT_SOURCE_DIR}/test/run-c-api-examples.py --bindir $<TARGET_FILE_DIR:wat2wasm>
    WORKING_DIRECTORY ${WABT_SOURCE_DIR}
    USES_TERMINAL
  )

  add_custom_target(check DEPENDS run-unittests run-tests run-c-api-tests)

  function(c_api_example NAME)
    set(EXENAME wasm-c-api-${NAME})
    add_executable(${EXENAME} third_party/wasm-c-api/example/${NAME}.c)
    if (COMPILER_IS_MSVC)
      set_target_properties(${EXENAME} PROPERTIES COMPILE_FLAGS "-wd4311")
    else ()
      set_target_properties(${EXENAME} PROPERTIES COMPILE_FLAGS "-std=gnu11 -Wno-pointer-to-int-cast")
    endif ()

    target_link_libraries(${EXENAME} wasm Threads::Threads)
    add_custom_target(${EXENAME}-copy-to-bin ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory ${WABT_SOURCE_DIR}/bin
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXENAME}> ${WABT_SOURCE_DIR}/bin/
      COMMAND ${CMAKE_COMMAND} -E copy ${WABT_SOURCE_DIR}/third_party/wasm-c-api/example/${NAME}.wasm $<TARGET_FILE_DIR:${EXENAME}>/
      COMMAND ${CMAKE_COMMAND} -E copy ${WABT_SOURCE_DIR}/third_party/wasm-c-api/example/${NAME}.wasm ${WABT_SOURCE_DIR}/bin/
      DEPENDS ${EXENAME}
    )
    add_dependencies(run-c-api-tests ${EXENAME})
  endfunction()

  c_api_example(callback)
  c_api_example(finalize)
  c_api_example(global)
  c_api_example(hello)
  c_api_example(hostref)
  c_api_example(multi)
  c_api_example(memory)
  c_api_example(reflect)
  c_api_example(serialize)
  c_api_example(start)
  c_api_example(table)
  c_api_example(trap)
  if (CMAKE_USE_PTHREADS_INIT)
    c_api_example(threads)
  endif ()
endif ()

# install
if (WABT_INSTALL_RULES AND (BUILD_TOOLS OR BUILD_TESTS))
  install(
    TARGETS ${WABT_EXECUTABLES}
    COMPONENT wabt-runtime
  )
  if (UNIX)
    install(
      DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/man/"
      DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
      COMPONENT wabt-documentation
      FILES_MATCHING PATTERN "*.1"
    )
  endif ()
endif ()

if (EMSCRIPTEN)
  # flags for all emscripten builds

  # exceptions are never needed
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

  # wabt.js

  # just dump everything into one binary so we can reference it from JavaScript
  add_definitions(-Wno-warn-absolute-paths)
  add_executable(libwabtjs src/emscripten-helpers.cc)
  add_dependencies(everything libwabtjs)
  target_link_libraries(libwabtjs wabt)
  set_target_properties(libwabtjs PROPERTIES OUTPUT_NAME libwabt)

  set(WABT_POST_JS ${WABT_SOURCE_DIR}/src/wabt.post.js)
  set(EMSCRIPTEN_EXPORTED_JSON ${WABT_SOURCE_DIR}/src/emscripten-exported.json)

  set(LIBWABT_LINK_FLAGS
    -s SINGLE_FILE
    --post-js ${WABT_POST_JS}
    -s EXPORTED_FUNCTIONS=\"@${EMSCRIPTEN_EXPORTED_JSON}\"
    -s RESERVED_FUNCTION_POINTERS=10
    -s NO_EXIT_RUNTIME=1
    -s ALLOW_MEMORY_GROWTH=1
    -s MODULARIZE=1
    -s EXPORT_NAME=\"'WabtModule'\"
    -s WASM=0
    -Oz
  )
  string(REPLACE ";" " " LIBWABT_LINK_FLAGS_STR "${LIBWABT_LINK_FLAGS}")

  set_target_properties(libwabtjs
    PROPERTIES
    LINK_FLAGS "${LIBWABT_LINK_FLAGS_STR}"
    LINK_DEPENDS "${WABT_POST_JS};${EMSCRIPTEN_EXPORTED_JSON}"
  )
endif ()

# Create find_package configuration files
if (WABT_INSTALL_RULES)
  include(CMakePackageConfigHelpers)

  set(WABT_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/wabt"
      CACHE STRING "Path to wabt CMake files")

  install(
    EXPORT wabt-targets
    DESTINATION "${WABT_INSTALL_CMAKEDIR}"
    NAMESPACE wabt::
    FILE wabt-targets.cmake
    COMPONENT wabt-development
  )

  configure_package_config_file(
    scripts/wabt-config.cmake.in
    wabt-config.cmake
    INSTALL_DESTINATION "${WABT_INSTALL_CMAKEDIR}"
    NO_SET_AND_CHECK_MACRO
  )

  write_basic_package_version_file(
    wabt-config-version.cmake COMPATIBILITY ExactVersion
  )

  install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/wabt-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/wabt-config-version.cmake"
    DESTINATION "${WABT_INSTALL_CMAKEDIR}"
    COMPONENT wabt-development
  )
endif ()

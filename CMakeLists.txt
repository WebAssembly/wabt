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

cmake_minimum_required(VERSION 3.1.0)
project(WABT LANGUAGES C CXX VERSION 1.0.29)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif (POLICY CMP0077)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check if wabt is being used directly or via add_subdirectory, FetchContent, etc
set(WABT_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(WABT_MASTER_PROJECT ON)
endif()

# For git users, attempt to generate a more useful version string
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  find_package(Git QUIET REQUIRED)
  execute_process(COMMAND
          "${GIT_EXECUTABLE}" --git-dir=${CMAKE_CURRENT_SOURCE_DIR}/.git describe --tags
          RESULT_VARIABLE
              GIT_VERSION_RESULT
          ERROR_VARIABLE
              GIT_VERSION_ERROR
          OUTPUT_VARIABLE
              GIT_VERSION
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (${GIT_VERSION_RESULT})
    # Don't issue warning if we aren't the master project;
    # just assume that whoever included us knows the version they are getting
    if (WABT_MASTER_PROJECT)
      message(WARNING "${GIT_VERSION_ERROR} Error running git describe to determine version")
    endif()
  else ()
    set(CMAKE_PROJECT_VERSION "${CMAKE_PROJECT_VERSION} (${GIT_VERSION})")
  endif ()
endif ()

if (NOT "${CMAKE_PROJECT_VERSION}")
  set(CMAKE_PROJECT_VERSION ${PROJECT_VERSION})
endif()

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

configure_file(
  ${WABT_SOURCE_DIR}/src/config.h.in
  ${WABT_BINARY_DIR}/config.h
)

include_directories(${WABT_SOURCE_DIR} ${WABT_BINARY_DIR})

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
        elseif (${FILE_OUTPUT} MATCHES "IBM S/390")
          set(TARGET_ARCH "s390x")
        else ()
          message(WARNING "Unknown target architecture!")
        endif ()
      else ()
        message(WARNING "Error running `file` command on dummy executable")
      endif ()
    else ()
      message(WARNING "Error compiling dummy.c file")
    endif ()

    if (TARGET_ARCH STREQUAL "i386")
      # wasm doesn't allow for x87 floating point math
      add_definitions(-msse2 -mfpmath=sse)
    endif ()
    if (TARGET_ARCH STREQUAL "s390x")
      add_definitions(-DWABT_BIG_ENDIAN=1)
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

set(WABT_LIBRARY_SRC
  ${WABT_BINARY_DIR}/config.h

  src/apply-names.h
  src/apply-names.cc
  src/binary.h
  src/binary.cc
  src/binary-reader.h
  src/binary-reader.cc
  src/binary-reader-ir.h
  src/binary-reader-ir.cc
  src/binary-reader-logging.h
  src/binary-reader-logging.cc
  src/binary-writer.h
  src/binary-writer.cc
  src/binary-writer-spec.h
  src/binary-writer-spec.cc
  src/binding-hash.h
  src/binding-hash.cc
  src/color.h
  src/color.cc
  src/common.h
  src/common.cc
  src/config.h.in
  src/config.cc
  src/decompiler.h
  src/decompiler-ast.h
  src/decompiler-ls.h
  src/decompiler-naming.h
  src/decompiler.cc
  src/error-formatter.h
  src/error-formatter.cc
  src/expr-visitor.h
  src/expr-visitor.cc
  src/feature.h
  src/feature.cc
  src/filenames.h
  src/filenames.cc
  src/generate-names.h
  src/generate-names.cc
  src/ir.h
  src/ir.cc
  src/ir-util.h
  src/ir-util.cc
  src/leb128.h
  src/leb128.cc
  src/lexer-source.h
  src/lexer-source.cc
  src/lexer-source-line-finder.h
  src/lexer-source-line-finder.cc
  src/literal.h
  src/literal.cc
  src/opcode.h
  src/opcode.cc
  src/opcode-code-table.h
  src/opcode-code-table.c
  src/option-parser.h
  src/option-parser.cc
  src/resolve-names.h
  src/resolve-names.cc
  src/shared-validator.h
  src/shared-validator.cc
  src/stream.h
  src/stream.cc
  src/string-util.h
  src/token.h
  src/token.cc
  src/tracing.h
  src/tracing.cc
  src/type.h
  src/type-checker.h
  src/type-checker.cc
  src/utf8.h
  src/utf8.cc
  src/validator.h
  src/validator.cc
  src/wast-lexer.h
  src/wast-lexer.cc
  src/wast-parser.h
  src/wast-parser.cc
  src/wat-writer.h
  src/wat-writer.cc

  # TODO(binji): Move this into its own library?
  src/interp/binary-reader-interp.h
  src/interp/binary-reader-interp.cc
  src/interp/interp.h
  src/interp/interp.cc
  src/interp/interp-inl.h
  src/interp/interp-math.h
  src/interp/interp-util.h
  src/interp/interp-util.cc
  src/interp/istream.h
  src/interp/istream.cc
)

add_library(wabt STATIC ${WABT_LIBRARY_SRC})

IF (NOT WIN32)
  add_library(wasm-rt-impl STATIC wasm2c/wasm-rt-impl.c wasm2c/wasm-rt-impl.h)
  install(TARGETS wasm-rt-impl DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES wasm2c/wasm-rt.h wasm2c/wasm-rt-impl.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
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
  target_link_libraries(wasm wabt)
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

# Python 3.5 is the version shipped in Ubuntu Xenial
find_package(PythonInterp 3.5)
if(BUILD_TESTS AND (NOT PYTHONINTERP_FOUND))
  set(BUILD_TESTS OFF)
  message(WARNING "Skipping tests. Python 3 is required for wabt testing. Please install python3 to run tests.")
endif()

find_package(Threads)
if (BUILD_TESTS)
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

  if (NOT CMAKE_VERSION VERSION_LESS "3.2")
    set(USES_TERMINAL USES_TERMINAL)
  endif ()

  # test running
  set(RUN_TESTS_PY ${WABT_SOURCE_DIR}/test/run-tests.py)

  add_custom_target(run-tests
    COMMAND ${PYTHON_EXECUTABLE} ${RUN_TESTS_PY} --bindir $<TARGET_FILE_DIR:wat2wasm>
    DEPENDS ${WABT_EXECUTABLES}
    WORKING_DIRECTORY ${WABT_SOURCE_DIR}
    ${USES_TERMINAL}
  )

  add_custom_target(run-unittests
    COMMAND $<TARGET_FILE:wabt-unittests>
    DEPENDS wabt-unittests
    WORKING_DIRECTORY ${WABT_SOURCE_DIR}
    ${USES_TERMINAL}
  )

  add_custom_target(run-c-api-tests
    COMMAND ${PYTHON_EXECUTABLE} ${WABT_SOURCE_DIR}/test/run-c-api-examples.py --bindir $<TARGET_FILE_DIR:wat2wasm>
    WORKING_DIRECTORY ${WABT_SOURCE_DIR}
    ${USES_TERMINAL}
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
  if (NOT WIN32)
    # depends on pthreads
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    c_api_example(threads)
  endif ()
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

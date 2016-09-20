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

.SUFFIXES:

MAKEFILE_NAME := $(lastword $(MAKEFILE_LIST))
ROOT_DIR := $(dir $(abspath $(MAKEFILE_NAME)))

USE_NINJA ?= 0
FUZZ_BIN_DIR ?= ${ROOT_DIR}/afl-fuzz
GCC_FUZZ_CC := ${FUZZ_BIN_DIR}/afl-gcc
GCC_FUZZ_CXX := ${FUZZ_BIN_DIR}/afl-g++
EMSCRIPTEN_DIR ?= ${ROOT_DIR}/emscripten

DEFAULT_COMPILER = CLANG
DEFAULT_BUILD_TYPE = DEBUG

COMPILERS := GCC GCC_I686 GCC_FUZZ CLANG EMSCRIPTEN
BUILD_TYPES := DEBUG RELEASE
SANITIZERS := ASAN MSAN LSAN UBSAN
CONFIGS := NORMAL $(SANITIZERS) NO_RE2C_BISON NO_TESTS
EXECUTABLES := wast2wasm wasm2wast wasm-interp hexfloat_test

# directory names
GCC_DIR := gcc/
GCC_I686_DIR := gcc-i686/
GCC_FUZZ_DIR := gcc-fuzz/
CLANG_DIR := clang/
EMSCRIPTEN_DIR := emscripten/
DEBUG_DIR := Debug/
RELEASE_DIR := Release/
NORMAL_DIR :=
ASAN_DIR := asan/
MSAN_DIR := msan/
LSAN_DIR := lsan/
UBSAN_DIR := ubsan/
NO_RE2C_BISON_DIR := no-re2c-bison/
NO_TESTS_DIR := no-tests/

# CMake flags
GCC_FLAG := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
GCC_I686_FLAG := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
	-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
GCC_FUZZ_FLAG := -DCMAKE_C_COMPILER=${GCC_FUZZ_CC} -DCMAKE_CXX_COMPILER=${GCC_FUZZ_CXX}
CLANG_FLAG := -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
EMSCRIPTEN_FLAG := -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_DIR}/cmake/Modules/Platform/Emscripten.cmake
DEBUG_FLAG := -DCMAKE_BUILD_TYPE=Debug
RELEASE_FLAG := -DCMAKE_BUILD_TYPE=Release
NORMAL_FLAG :=
ASAN_FLAG := -DUSE_ASAN=ON
MSAN_FLAG := -DUSE_MSAN=ON
LSAN_FLAG := -DUSE_LSAN=ON
UBSAN_FLAG := -DUSE_UBSAN=ON
NO_RE2C_BISON_FLAG := -DRUN_BISON=OFF -DRUN_RE2C=OFF
NO_TESTS_FLAG := -DBUILD_TESTS=OFF

# make target prefixes
GCC_PREFIX := gcc
GCC_I686_PREFIX := gcc-i686
GCC_FUZZ_PREFIX := gcc-fuzz
CLANG_PREFIX := clang
EMSCRIPTEN_PREFIX := emscripten
DEBUG_PREFIX := -debug
RELEASE_PREFIX := -release
NORMAL_PREFIX :=
ASAN_PREFIX := -asan
MSAN_PREFIX := -msan
LSAN_PREFIX := -lsan
UBSAN_PREFIX := -ubsan
NO_RE2C_BISON_PREFIX := -no-re2c-bison
NO_TESTS_PREFIX := -no-tests

ifeq ($(USE_NINJA),1)
BUILD := ninja
BUILD_FILE := build.ninja
GENERATOR := Ninja
else
BUILD := $(MAKE) --no-print-directory
BUILD_FILE := Makefile
GENERATOR := "Unix Makefiles"
endif

CMAKE_DIR = out/$($(1)_DIR)$($(2)_DIR)$($(3)_DIR)
EXE_TARGET = $($(1)_PREFIX)$($(2)_PREFIX)$($(3)_PREFIX)
TEST_TARGET = test-$($(1)_PREFIX)$($(2)_PREFIX)$($(3)_PREFIX)

define DEFAULT
.PHONY: $(3)$($(4)_PREFIX) test$($(4)_PREFIX)
$(3)$($(4)_PREFIX): $(call EXE_TARGET,$(1),$(2),$(4))
	ln -sf ../$(call CMAKE_DIR,$(1),$(2),$(4))$(3) out/$(3)$($(4)_PREFIX)

test$($(4)_PREFIX): $(call TEST_TARGET,$(1),$(2),$(4))
test-everything: test$($(4)_PREFIX)
endef

define CMAKE
$(call CMAKE_DIR,$(1),$(2),$(3)):
	mkdir -p $(call CMAKE_DIR,$(1),$(2),$(3))

$(call CMAKE_DIR,$(1),$(2),$(3))$$(BUILD_FILE): | $(call CMAKE_DIR,$(1),$(2),$(3))
	cd $(call CMAKE_DIR,$(1),$(2),$(3)) && \
	cmake -G $$(GENERATOR) $$(ROOT_DIR) $$($(1)_FLAG) $$($(2)_FLAG) $$($(3)_FLAG)
endef

define EXE
.PHONY: $(call EXE_TARGET,$(1),$(2),$(3))
$(call EXE_TARGET,$(1),$(2),$(3)): $(call CMAKE_DIR,$(1),$(2),$(3))$$(BUILD_FILE)
	$$(BUILD) -C $(call CMAKE_DIR,$(1),$(2),$(3)) all
endef

define TEST
.PHONY: $(call TEST_TARGET,$(1),$(2),$(3))
$(call TEST_TARGET,$(1),$(2),$(3)): $(call CMAKE_DIR,$(1),$(2),$(3))$$(BUILD_FILE)
	$$(BUILD) -C $(call CMAKE_DIR,$(1),$(2),$(3)) run-tests
endef

.PHONY: all
all: ${EXECUTABLES}

.PHONY: clean
clean:
	rm -rf out

.PHONY: test-everything
test-everything:

.PHONY: update-bison update-re2c
update-bison: src/prebuilt/wasm-ast-parser-gen.c
update-re2c: src/prebuilt/wasm-ast-lexer-gen.c

src/prebuilt/wasm-ast-parser-gen.c: src/wasm-ast-parser.y
	bison -o $@ $< --defines=src/prebuilt/wasm-ast-parser-gen.h

src/prebuilt/wasm-ast-lexer-gen.c: src/wasm-ast-lexer.c
	re2c --no-generation-date -bc -o $@ $<

# defaults with simple names
$(foreach EXECUTABLE,$(EXECUTABLES), \
	$(eval $(call DEFAULT,$(DEFAULT_COMPILER),$(DEFAULT_BUILD_TYPE),$(EXECUTABLE),NORMAL)) \
	$(foreach SANITIZER,$(SANITIZERS), \
		$(eval $(call DEFAULT,$(DEFAULT_COMPILER),$(DEFAULT_BUILD_TYPE),$(EXECUTABLE),$(SANITIZER)))))

# running CMake
$(foreach CONFIG,$(CONFIGS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call CMAKE,$(COMPILER),$(BUILD_TYPE),$(CONFIG))))))

# building
$(foreach CONFIG,$(CONFIGS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call EXE,$(COMPILER),$(BUILD_TYPE),$(CONFIG))))))

# test running
$(foreach CONFIG,$(CONFIGS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call TEST,$(COMPILER),$(BUILD_TYPE),$(CONFIG))))))

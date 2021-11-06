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

USE_NINJA ?= 1
EMSCRIPTEN_DIR ?= $(dir $(shell which emcc))
CMAKE_CMD ?= cmake

DEFAULT_SUFFIX = clang-debug

COMPILERS := GCC GCC_I686 CLANG CLANG_I686 EMCC
BUILD_TYPES := DEBUG RELEASE
SANITIZERS := ASAN MSAN LSAN UBSAN FUZZ
CONFIGS := NORMAL $(SANITIZERS) COV NO_TESTS

# directory names
GCC_DIR := gcc/
GCC_I686_DIR := gcc-i686/
CLANG_DIR := clang/
CLANG_I686_DIR := clang-i686/
EMCC_DIR := emscripten/
DEBUG_DIR := Debug/
RELEASE_DIR := Release/
NORMAL_DIR :=
ASAN_DIR := asan/
MSAN_DIR := msan/
LSAN_DIR := lsan/
UBSAN_DIR := ubsan/
FUZZ_DIR := fuzz/
COV_DIR := cov/
NO_TESTS_DIR := no-tests/

# CMake flags
GCC_FLAG := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
GCC_I686_FLAG := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
	-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
CLANG_FLAG := -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
CLANG_I686_FLAG := -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
	-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
EMCC_FLAG := -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_DIR}/cmake/Modules/Platform/Emscripten.cmake
DEBUG_FLAG := -DCMAKE_BUILD_TYPE=Debug
RELEASE_FLAG := -DCMAKE_BUILD_TYPE=Release
NORMAL_FLAG :=
ASAN_FLAG := -DUSE_ASAN=ON
MSAN_FLAG := -DUSE_MSAN=ON
LSAN_FLAG := -DUSE_LSAN=ON
UBSAN_FLAG := -DUSE_UBSAN=ON
FUZZ_FLAG := -DBUILD_FUZZ_TOOLS=ON
COV_FLAG := -DCODE_COVERAGE=ON
NO_TESTS_FLAG := -DBUILD_TESTS=OFF

# make target prefixes
GCC_PREFIX := gcc
GCC_I686_PREFIX := gcc-i686
CLANG_PREFIX := clang
CLANG_I686_PREFIX := clang-i686
EMCC_PREFIX := emscripten
DEBUG_PREFIX := -debug
RELEASE_PREFIX := -release
NORMAL_PREFIX :=
ASAN_PREFIX := -asan
MSAN_PREFIX := -msan
LSAN_PREFIX := -lsan
UBSAN_PREFIX := -ubsan
FUZZ_PREFIX := -fuzz
COV_PREFIX := -cov
NO_TESTS_PREFIX := -no-tests

ifeq ($(USE_NINJA),1)
BUILD_CMD := ninja
BUILD_FILE := build.ninja
GENERATOR := Ninja
else
BUILD_CMD := +$(MAKE) --no-print-directory
BUILD_FILE := Makefile
GENERATOR := "Unix Makefiles"
endif

CMAKE_DIR = out/$($(1)_DIR)$($(2)_DIR)$($(3)_DIR)
BUILD_TARGET = $($(1)_PREFIX)$($(2)_PREFIX)$($(3)_PREFIX)
INSTALL_TARGET = install-$($(1)_PREFIX)$($(2)_PREFIX)$($(3)_PREFIX)
TEST_TARGET = test-$($(1)_PREFIX)$($(2)_PREFIX)$($(3)_PREFIX)

define CMAKE
$(call CMAKE_DIR,$(1),$(2),$(3)):
	mkdir -p $(call CMAKE_DIR,$(1),$(2),$(3))

$(call CMAKE_DIR,$(1),$(2),$(3))$$(BUILD_FILE): | $(call CMAKE_DIR,$(1),$(2),$(3))
	cd $(call CMAKE_DIR,$(1),$(2),$(3)) && \
	$$(CMAKE_CMD) -G $$(GENERATOR) $$(ROOT_DIR) $$($(1)_FLAG) $$($(2)_FLAG) $$($(3)_FLAG)
endef

define BUILD
.PHONY: $(call BUILD_TARGET,$(1),$(2),$(3))
$(call BUILD_TARGET,$(1),$(2),$(3)): $(call CMAKE_DIR,$(1),$(2),$(3))$$(BUILD_FILE)
	$$(BUILD_CMD) -C $(call CMAKE_DIR,$(1),$(2),$(3)) all
endef

define INSTALL
.PHONY: $(call INSTALL_TARGET,$(1),$(2),$(3))
$(call INSTALL_TARGET,$(1),$(2),$(3)): $(call CMAKE_DIR,$(1),$(2),$(3))$$(BUILD_FILE)
	$$(BUILD_CMD) -C $(call CMAKE_DIR,$(1),$(2),$(3)) install
endef

define TEST
.PHONY: $(call TEST_TARGET,$(1),$(2),$(3))
$(call TEST_TARGET,$(1),$(2),$(3)): $(call CMAKE_DIR,$(1),$(2),$(3))$$(BUILD_FILE)
	$$(BUILD_CMD) -C $(call CMAKE_DIR,$(1),$(2),$(3)) check
test-everything: $(CALL TEST_TARGET,$(1),$(2),$(3))
endef

.PHONY: all install test
all: $(DEFAULT_SUFFIX)
install: install-$(DEFAULT_SUFFIX)
test: test-$(DEFAULT_SUFFIX)

.PHONY: clean
clean:
	rm -rf out

.PHONY: test-everything
test-everything:

.PHONY: update-gperf
update-gperf: src/prebuilt/lexer-keywords.cc

src/prebuilt/lexer-keywords.cc: src/lexer-keywords.txt
	gperf -m 50 -L C++ -N InWordSet -E -t -c --output-file=$@ $<

.PHONY: update-wasm2c
update-wasm2c: src/prebuilt/wasm2c.include.c src/prebuilt/wasm2c.include.h

src/prebuilt/wasm2c.include.c: src/wasm2c.c.tmpl
	src/wasm2c_tmpl.py -o $@ $<

src/prebuilt/wasm2c.include.h: src/wasm2c.h.tmpl
	src/wasm2c_tmpl.py -o $@ $<

.PHONY: demo
demo: emscripten-release
	cp out/emscripten/Release/libwabt.js docs/demo

# running CMake
$(foreach CONFIG,$(CONFIGS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call CMAKE,$(COMPILER),$(BUILD_TYPE),$(CONFIG))))))

# building
$(foreach CONFIG,$(CONFIGS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call BUILD,$(COMPILER),$(BUILD_TYPE),$(CONFIG))))))

# installing
$(foreach CONFIG,$(CONFIGS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call INSTALL,$(COMPILER),$(BUILD_TYPE),$(CONFIG))))))

# test running
$(foreach CONFIG,$(CONFIGS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call TEST,$(COMPILER),$(BUILD_TYPE),$(CONFIG))))))

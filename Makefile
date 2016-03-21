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

USE_NINJA ?= 0

DEFAULT_COMPILER = CLANG
DEFAULT_BUILD_TYPE = DEBUG

COMPILERS := GCC GCC_I686 CLANG
BUILD_TYPES := DEBUG RELEASE
SANITIZERS := NO ASAN MSAN LSAN
EXECUTABLES := sexpr-wasm hexfloat_test

GCC_DEBUG_DIR := out/gcc/Debug
GCC_DEBUG_NO_FLEX_BISON_DIR := out/gcc/Debug-no-flex-bison
GCC_DEBUG_NO_TESTS_DIR := out/gcc/Debug-no-tests
GCC_RELEASE_DIR := out/gcc/Release
GCC_I686_DEBUG_DIR := out/gcc-i686/Debug
GCC_I686_RELEASE_DIR := out/gcc-i686/Release
CLANG_DEBUG_DIR := out/clang/Debug
CLANG_RELEASE_DIR := out/clang/Release
CLANG_DEBUG_NO_TESTS_DIR := out/clang/Debug-no-tests

DEBUG_FLAG := -DCMAKE_BUILD_TYPE=Debug
RELEASE_FLAG := -DCMAKE_BUILD_TYPE=Release
GCC_FLAG := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
GCC_I686_FLAG := -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
	-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
CLANG_FLAG := -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

GCC_DEBUG_PREFIX := gcc-debug
GCC_DEBUG_NO_FLEX_BISON_PREFIX := gcc-debug-no-flex-bison
GCC_DEBUG_NO_TESTS_PREFIX := gcc-debug-no-tests
GCC_RELEASE_PREFIX := gcc-release
GCC_I686_DEBUG_PREFIX := gcc-i686-debug
GCC_I686_RELEASE_PREFIX := gcc-i686-release
CLANG_DEBUG_PREFIX := clang-debug
CLANG_RELEASE_PREFIX := clang-release
CLANG_DEBUG_NO_TESTS_PREFIX := clang-debug-no-tests

NO_SUFFIX :=
ASAN_SUFFIX := -asan
MSAN_SUFFIX := -msan
LSAN_SUFFIX := -lsan

ifeq ($(USE_NINJA),1)
BUILD := ninja
BUILD_FILE := build.ninja
GENERATOR := Ninja
else
BUILD := $(MAKE) --no-print-directory
BUILD_FILE := Makefile
GENERATOR := "Unix Makefiles"
endif

define DEFAULT
.PHONY: $(3)$$($(4)_SUFFIX) test$$($(4)_SUFFIX)
$(3)$$($(4)_SUFFIX): $$($(1)_$(2)_PREFIX)$$($(4)_SUFFIX)
	ln -sf ../$$($(1)_$(2)_DIR)/$(3)$$($(4)_SUFFIX) out/$(3)$$($(4)_SUFFIX)

test$$($(4)_SUFFIX): test-$$($(1)_$(2)_PREFIX)$$($(4)_SUFFIX)

test-everything: test$$($(4)_SUFFIX)
endef

define CMAKE
$$($(1)_$(2)_DIR)/:
	mkdir -p $$($(1)_$(2)_DIR)

$$($(1)_$(2)_DIR)/$$(BUILD_FILE): | $$($(1)_$(2)_DIR)/
	cd $$($(1)_$(2)_DIR) && cmake -G $$(GENERATOR) ../../.. $$($(1)_FLAG) $$($(2)_FLAG) $(3)
endef

define EXE
.PHONY: $$($(1)_$(2)_PREFIX)$$($(3)_SUFFIX)
$$($(1)_$(2)_PREFIX)$$($(3)_SUFFIX): $$($(1)_$(2)_DIR)/$$(BUILD_FILE)
	$$(BUILD) -C $$($(1)_$(2)_DIR) all$$($(3)_SUFFIX)
endef

define TEST
.PHONY: test-$$($(1)_$(2)_PREFIX)$$($(3)_SUFFIX)
test-$$($(1)_$(2)_PREFIX)$$($(3)_SUFFIX): $$($(1)_$(2)_DIR)/$$(BUILD_FILE)
	$$(BUILD) -C $$($(1)_$(2)_DIR) run-tests$$($(3)_SUFFIX)
endef

.PHONY: all
all: ${EXECUTABLES}

.PHONY: clean
clean:
	rm -rf out

.PHONY: test-everything
test-everything:

.PHONY: update-bison update-flex
update-bison: src/prebuilt/wasm-bison-parser.c
update-flex: src/prebuilt/wasm-flex-lexer.c

src/prebuilt/wasm-bison-parser.c: src/wasm-bison-parser.y
	bison -o $@ $< --defines=src/prebuilt/wasm-bison-parser.h

src/prebuilt/wasm-flex-lexer.c: src/wasm-flex-lexer.l
	flex -o $@ $<

# defaults with simple names
$(foreach SANITIZER,$(SANITIZERS), \
	$(foreach EXECUTABLE,$(EXECUTABLES), \
		$(eval $(call DEFAULT,$(DEFAULT_COMPILER),$(DEFAULT_BUILD_TYPE),$(EXECUTABLE),$(SANITIZER)))))

# running CMake
$(foreach COMPILER,$(COMPILERS), \
	$(foreach BUILD_TYPE,$(BUILD_TYPES), \
		$(eval $(call CMAKE,$(COMPILER),$(BUILD_TYPE)))))

# building
$(foreach SANITIZER,$(SANITIZERS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call EXE,$(COMPILER),$(BUILD_TYPE),$(SANITIZER))))))

# test running
$(foreach SANITIZER,$(SANITIZERS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call TEST,$(COMPILER),$(BUILD_TYPE),$(SANITIZER))))))

# One-off build target for running w/out flex + bison
$(eval $(call CMAKE,GCC,DEBUG_NO_FLEX_BISON,-DRUN_FLEX_BISON=OFF))
$(eval $(call EXE,GCC,DEBUG_NO_FLEX_BISON,NO))
$(eval $(call TEST,GCC,DEBUG_NO_FLEX_BISON,NO))

# One-off build target for running w/out gtest
$(eval $(call CMAKE,CLANG,DEBUG_NO_TESTS,-DBUILD_TESTS=OFF))
$(eval $(call EXE,CLANG,DEBUG_NO_TESTS,NO))
$(eval $(call TEST,CLANG,DEBUG_NO_TESTS,NO))

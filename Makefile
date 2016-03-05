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

DEFAULT_COMPILER = CLANG
DEFAULT_BUILD_TYPE = DEBUG

COMPILERS := GCC CLANG
BUILD_TYPES := DEBUG RELEASE
SANITIZERS := NO ASAN MSAN LSAN

GCC_DEBUG_DIR := out/gcc/Debug
GCC_RELEASE_DIR := out/gcc/Release
CLANG_DEBUG_DIR := out/clang/Debug
CLANG_RELEASE_DIR := out/clang/Release

DEBUG_FLAG := -DCMAKE_BUILD_TYPE=Debug
RELEASE_FLAG := -DCMAKE_BUILD_TYPE=Release
GCC_FLAG := -DCMAKE_C_COMPILER=gcc
CLANG_FLAG := -DCMAKE_C_COMPILER=clang

GCC_DEBUG_PREFIX := gcc-debug
GCC_RELEASE_PREFIX := gcc-release
CLANG_DEBUG_PREFIX := clang-debug
CLANG_RELEASE_PREFIX := clang-release

NO_SUFFIX :=
ASAN_SUFFIX := -asan
MSAN_SUFFIX := -msan
LSAN_SUFFIX := -lsan

define DEFAULT
.PHONY: $(3)$$($(4)_SUFFIX) test$$($(4)_SUFFIX)
$(3)$$($(4)_SUFFIX): $$($(1)_$(2)_PREFIX)-$(3)$$($(4)_SUFFIX)
	ln -sf ../$$($(1)_$(2)_DIR)/$(3)$$($(4)_SUFFIX) out/$(3)$$($(4)_SUFFIX)

test$$($(4)_SUFFIX): test-$$($(1)_$(2)_PREFIX)$$($(4)_SUFFIX)

test-everything: test$$($(4)_SUFFIX)
endef

define CMAKE
$$($(1)_$(2)_DIR)/:
	mkdir -p $$($(1)_$(2)_DIR)

$$($(1)_$(2)_DIR)/Makefile: | $$($(1)_$(2)_DIR)/
	cd $$($(1)_$(2)_DIR) && cmake ../../.. $$($(1)_FLAG) $$($(2)_FLAG)
endef

define EXE
.PHONY: $$($(1)_$(2)_PREFIX)-$(3)$$($(4)_SUFFIX)
$$($(1)_$(2)_PREFIX)-$(3)$$($(4)_SUFFIX): $$($(1)_$(2)_DIR)/Makefile
	$$(MAKE) -C $$($(1)_$(2)_DIR) $(3)$$($(4)_SUFFIX)
endef

define TEST
.PHONY: test-$$($(1)_$(2)_PREFIX)$$($(3)_SUFFIX)
test-$$($(1)_$(2)_PREFIX)$$($(3)_SUFFIX): $$($(1)_$(2)_DIR)/Makefile
	$$(MAKE) -C $$($(1)_$(2)_DIR) test$$($(3)_SUFFIX)
endef

.PHONY: all
all: sexpr-wasm

.PHONY: test-everything
test-everything:

# defaults with simple names
$(foreach SANITIZER,$(SANITIZERS), \
	$(eval $(call DEFAULT,$(DEFAULT_COMPILER),$(DEFAULT_BUILD_TYPE),sexpr-wasm,$(SANITIZER))))

# running CMake
$(foreach COMPILER,$(COMPILERS), \
	$(foreach BUILD_TYPE,$(BUILD_TYPES), \
		$(eval $(call CMAKE,$(COMPILER),$(BUILD_TYPE)))))

# sexpr-wasm builds
$(foreach SANITIZER,$(SANITIZERS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call EXE,$(COMPILER),$(BUILD_TYPE),sexpr-wasm,$(SANITIZER))))))

# test running
$(foreach SANITIZER,$(SANITIZERS), \
	$(foreach COMPILER,$(COMPILERS), \
		$(foreach BUILD_TYPE,$(BUILD_TYPES), \
			$(eval $(call TEST,$(COMPILER),$(BUILD_TYPE),$(SANITIZER))))))

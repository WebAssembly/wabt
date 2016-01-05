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

ALL = sexpr-wasm
EVERYHING = $(ALL) sexpr-wasm-asan sexpr-wasm-msan sexpr-wasm-lsan
CFLAGS = -Wall -Werror -g -Wno-unused-function -Wno-return-type
DEPEND_FLAGS = -MMD -MP -MF $(patsubst %.o,%.d,$@)
LIBS =
ASAN_FLAGS = -fsanitize=address
MSAN_FLAGS = -fsanitize=memory
LSAN_FLAGS = -fsanitize=leak

SRCS = wasm.c sexpr-wasm.c wasm-parser.c wasm-lexer.c wasm-vector.c \
			 wasm-check.c wasm-writer.c wasm-binary-writer.c
OBJS = $(addprefix out/,$(patsubst %.c,%.o,$(SRCS)))
ASAN_OBJS = $(addprefix out/,$(patsubst %.c,%.asan.o,$(SRCS)))
MSAN_OBJS = $(addprefix out/,$(patsubst %.c,%.msan.o,$(SRCS)))
LSAN_OBJS = $(addprefix out/,$(patsubst %.c,%.lsan.o,$(SRCS)))

.PHONY: all
all: $(addprefix out/,$(ALL))

.PHONY: everything
everything: $(addprefix out/,$(EVERYHING))

out:
	mkdir $@

src/wasm-lexer.c src/wasm-lexer.h: src/wasm-lexer.l
	flex -o src/wasm-lexer.c $<

src/wasm-parser.c src/wasm-parser.h: src/wasm-parser.y
	bison -o src/wasm-parser.c --defines=src/wasm-parser.h $<

$(OBJS): out/%.o: src/%.c | out
	$(CC) $(CFLAGS) -c -o $@ $(DEPEND_FLAGS) $<
out/sexpr-wasm: $(OBJS) | out
	$(CC) -o $@ $^ ${LIBS}

# ASAN
$(ASAN_OBJS): out/%.asan.o: src/%.c | out
	clang $(ASAN_FLAGS) $(CFLAGS) -c -o $@ $(DEPEND_FLAGS) $<
out/sexpr-wasm-asan: $(ASAN_OBJS) | out
	clang $(ASAN_FLAGS) -o $@ $^ ${LIBS}

# MSAN
$(MSAN_OBJS): out/%.msan.o: src/%.c | out
	clang $(MSAN_FLAGS) $(CFLAGS) -c -o $@ $(DEPEND_FLAGS) $<
out/sexpr-wasm-msan: $(MSAN_OBJS) | out
	clang $(MSAN_FLAGS) -o $@ $^ ${LIBS}

# LSAN
$(LSAN_OBJS): out/%.lsan.o: src/%.c | out
	clang $(LSAN_FLAGS) $(CFLAGS) -c -o $@ $(DEPEND_FLAGS) $<
out/sexpr-wasm-lsan: $(LSAN_OBJS) | out
	clang $(LSAN_FLAGS) -o $@ $^ ${LIBS}

-include $(OBJS:.o=.d) $(ASAN_OBJS:.o=.d) $(MSAN_OBJS:.o=.d) $(LSAN_OBJS:.o=.d)

#### TESTS ####
.PHONY: test
test: out/sexpr-wasm
	@python test/run-tests.py

.PHONY: test-asan
test-asan: out/sexpr-wasm-asan
	@python test/run-tests.py -e $<

.PHONY: test-msan
test-msan: out/sexpr-wasm-msan
	@python test/run-tests.py -e $<

.PHONY: test-lsan
test-lsan: out/sexpr-wasm-lsan
	@python test/run-tests.py -e $<

.PHONY: test-everything
test-everything: test test-asan test-msan test-lsan

#### CLEAN ####
.PHONY: clean
clean:
	rm -rf out

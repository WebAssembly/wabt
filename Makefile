.SUFFIXES:

ALL = sexpr-wasm
CC = gcc
CFLAGS = -Wall -Werror -g

.PHONY: all
all: $(addprefix out/,$(ALL))

out/:
	mkdir $@

out/sexpr-wasm: sexpr-wasm.c | out
	$(CC) $(CFLAGS) -o $@ $<

#### TESTS ####
TEST_EXES=$(shell python test/run-tests.py --list-exes)

.PHONY: test
test: $(TEST_EXES)
	@python test/run-tests.py

#### CLEAN ####
.PHONY: clean
clean:
	rm -rf out

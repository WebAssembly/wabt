.SUFFIXES:

ALL = sexpr-wasm
CFLAGS = -Wall -Werror -g

.PHONY: all
all: $(addprefix out/,$(ALL))

out/:
	mkdir $@

out/sexpr-wasm: sexpr-wasm.c hash.h | out
	$(CC) $(CFLAGS) -o $@ $<

hash.h: hash.txt
	gperf --compare-strncmp --readonly-tables --struct-type $< --output-file $@

#### TESTS ####
TEST_EXES=$(shell python test/run-tests.py --list-exes)

.PHONY: test
test: $(TEST_EXES)
	@python test/run-tests.py

#### CLEAN ####
.PHONY: clean
clean:
	rm -rf out

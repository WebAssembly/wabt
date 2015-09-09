.SUFFIXES:

ALL = sexpr-wasm
CFLAGS = -Wall -Werror -g

OBJS = out/sexpr-wasm.o out/parse.o
HEADERS = src/wasm.h src/wasm-parse.h

.PHONY: all
all: $(addprefix out/,$(ALL))

out/:
	mkdir $@
out/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<
out/sexpr-wasm: out $(OBJS)
	$(CC) -o $@ $(OBJS)

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

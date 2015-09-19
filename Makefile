.SUFFIXES:

ALL = sexpr-wasm
CFLAGS = -Wall -Werror -g

OBJS = out/sexpr-wasm.o out/wasm-parse.o out/wasm-gen.o

.PHONY: all
all: $(addprefix out/,$(ALL))

out/:
	mkdir $@
out/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ -MMD -MP -MF $(patsubst %.o,%.d,$@) $<
out/sexpr-wasm: out $(OBJS)
	$(CC) -o $@ $(OBJS)

src/hash.h: src/hash.txt
	gperf --compare-strncmp --readonly-tables --struct-type $< --output-file $@

-include $(OBJS:.o=.d)

#### TESTS ####
TEST_EXES=$(shell python test/run-tests.py --list-exes)

.PHONY: test
test: $(TEST_EXES)
	@python test/run-tests.py

#### CLEAN ####
.PHONY: clean
clean:
	rm -rf out

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

.PHONY: clean
clean:
	rm -rf out

# Use implicit rules for compiling C files.
CFLAGS=-I../..

all: rot13

clean:
	rm -rf rot13 rot13.wasm rot13.c *.o

rot13: main.o rot13.o ../../wasm-rt-impl.o -lm

rot13.wasm: rot13.wat ../../../bin/wat2wasm
	../../../bin/wat2wasm $< -o $@

rot13.c: rot13.wasm ../../../bin/wasm2c
	../../../bin/wasm2c $< -o $@

.PHONY: all clean

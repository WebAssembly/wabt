# Use implicit rules for compiling C files.
CFLAGS=-I../..
LDLIBS=-lm

all: rot13

clean:
	rm -rf rot13 rot13.wasm rot13.c rot13.h *.o

rot13: main.o rot13.o ../../wasm-rt-impl.o ../../wasm-rt-mem-impl.o

main.o: rot13.h

rot13.wasm: rot13.wat ../../../bin/wat2wasm
	../../../bin/wat2wasm $< -o $@

rot13.c rot13.h: rot13.wasm ../../../bin/wasm2c
	../../../bin/wasm2c $< -o rot13.c --disable-simd

.PHONY: all clean

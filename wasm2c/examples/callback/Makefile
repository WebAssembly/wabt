# Use implicit rules for compiling C files.
CFLAGS=-I../..
LDLIBS=-lm

all: callback

clean:
	rm -rf callback callback.wasm callback.c callback.h *.o

callback: main.o callback.o ../../wasm-rt-impl.o ../../wasm-rt-mem-impl.o

callback.wasm: callback.wat ../../../bin/wat2wasm
	../../../bin/wat2wasm --debug-names $< -o $@

callback.c: callback.wasm ../../../bin/wasm2c
	../../../bin/wasm2c $< -o $@ --disable-simd

.PHONY: all clean

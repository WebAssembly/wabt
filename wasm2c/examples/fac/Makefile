# Use implicit rules for compiling C files.
CFLAGS=-I../..
LDFLAGS=-lm

all: fac

clean:
	rm -rf fac fac.wasm fac.c *.o

fac: main.o fac.o ../../wasm-rt-impl.o
	$(CC) -o $@ $^ $(LDFLAGS)

fac.wasm: fac.wat ../../../bin/wat2wasm
	../../../bin/wat2wasm $< -o $@

fac.c: fac.wasm ../../../bin/wasm2c
	../../../bin/wasm2c $< -o $@ --disable-simd

.PHONY: all clean

# Use implicit rules for compiling C files.
CFLAGS=-I../..
rot13: main.o rot13.o ../../wasm-rt-impl.o

rot13.wasm: rot13.wat
	../../../bin/wat2wasm $< -o $@

rot13.c: rot13.wasm
	../../../bin/wasm2c $< -o $@

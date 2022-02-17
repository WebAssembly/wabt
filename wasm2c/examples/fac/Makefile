# Use implicit rules for compiling C files.
CFLAGS=-I../..
fac: main.o fac.o ../../wasm-rt-impl.o ../../wasm-rt-os-unix.o ../../wasm-rt-os-win.o ../../wasm-rt-wasi.o

fac.wasm: fac.wat
	../../../bin/wat2wasm $< -o $@

fac.c: fac.wasm
	../../../bin/wasm2c $< -o $@

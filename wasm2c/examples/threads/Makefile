#SANITIZERS=-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all
CFLAGS=-I../.. -g -O2 -Wall -Wextra -Wno-unused -Wno-unused-parameter -Wno-array-bounds -Wno-ignored-optimization-argument -Wno-tautological-constant-out-of-range-compare -Wno-infinite-recursion -fno-optimize-sibling-calls -frounding-math -fsignaling-nans ${SANITIZERS} -pthread
LDFLAGS=${SANITIZERS} -pthread

all: threads

threads: threads.o sample.o ../../wasm-rt-impl.o ../../wasm-rt-exceptions-impl.o -lm

clean:
	rm -rf threads sample.wasm sample.c sample.h *.o ../../*.o

sample.wasm: sample.wat ../../../bin/wat2wasm
	../../../bin/wat2wasm --debug-names $< -o $@

sample.c: sample.wasm ../../../bin/wasm2c
	../../../bin/wasm2c $< -o $@

threads.o: sample.c

.PHONY: all clean

WABT_ROOT=../../..
CC=clang
CFLAGS=-I$(WABT_ROOT)/wasm2c -I $(WABT_ROOT)/third_party/uvwasi/include/ -O3
CFLAGS_SEGUE=-DWASM_RT_ALLOW_SEGUE=1 -mfsgsbase
LDFLAGS=-L$(WABT_ROOT)/build/_deps/libuv-build -L$(WABT_ROOT)/build/third_party/uvwasi
LDLIBS=-luvwasi_a -luv_a -lm -lpthread

all: benchmark

clean:
	rm -rf dhrystone dhrystone.wasm dhrystone.c dhrystone.h

dhrystone.wasm: src/dhry_1.c src/dhry_2.c
	/opt/wasi-sdk/bin/clang -O3 $^ -o $@

dhrystone.c: dhrystone.wasm $(WABT_ROOT)/bin/wasm2c
	$(WABT_ROOT)/bin/wasm2c $< -o $@ --disable-simd

dhrystone_native: src/dhry_1.c src/dhry_2.c
	clang -O3 $^ -o $@

dhrystone: main.c dhrystone.c $(WABT_ROOT)/wasm2c/wasm-rt-impl.c $(WABT_ROOT)/wasm2c/wasm-rt-mem-impl.c
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

dhrystone_segue: main.c dhrystone.c $(WABT_ROOT)/wasm2c/wasm-rt-impl.c $(WABT_ROOT)/wasm2c/wasm-rt-mem-impl.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(CFLAGS_SEGUE) $^ -o $@ $(LDLIBS)

benchmark: dhrystone_native dhrystone dhrystone_segue
	@echo "Starting Dhrystone benchmark. (Smaller number is better)"
	@sleep 2
	@echo "Native"
	@./dhrystone_native | grep "one run through Dhrystone"
	@sleep 2
	@echo "Wasm"
	@./dhrystone | grep "one run through Dhrystone"
	@sleep 2
	@echo "Wasm+Segue"
	@./dhrystone_segue | grep "one run through Dhrystone"

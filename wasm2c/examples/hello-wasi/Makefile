WABT_ROOT=../../..
CC=clang
CFLAGS=-I$(WABT_ROOT)/wasm2c -I$(WABT_ROOT)/third_party/uvwasi/include -O3
LDFLAGS=-L$(WABT_ROOT)/build/_deps/libuv-build -L$(WABT_ROOT)/build/third_party/uvwasi
LDLIBS=-luvwasi_a -luv_a -lm -lpthread

all: hello-wasi

clean:
	rm -rf hello-wasi hello-wasi.wasm hello-wasi-wasm.c hello-wasi-wasm.h test.out *.o

hello-wasi.wasm: hello-wasi.c
	/opt/wasi-sdk/bin/clang -O3 $< -o $@

hello-wasi-wasm.c: hello-wasi.wasm $(WABT_ROOT)/bin/wasm2c
	$(WABT_ROOT)/bin/wasm2c $< -n hello_wasi -o $@

hello-wasi: main.c hello-wasi-wasm.c $(WABT_ROOT)/wasm2c/wasm-rt-uvwasi-adapter.c $(WABT_ROOT)/wasm2c/wasm-rt-impl.c $(WABT_ROOT)/wasm2c/wasm-rt-mem-impl.c
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

.PHONY: all clean

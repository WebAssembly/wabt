#assumes wasi-sdk installed system wide in default location
WASI_SDK_ROOT=/opt/wasi-sdk
WABT_ROOT=../../..

WASI_CLANG=$(WASI_SDK_ROOT)/bin/clang
WASI_SYSROOT=$(WASI_SDK_ROOT)/share/wasi-sysroot

#CFLAGS for compiling files to place nice with wasm2c
WASM_CFLAGS=-Wl,--export-all -Wl,--no-entry -Wl,--growable-table -Wl,--stack-first -Wl,-z,stack-size=1048576


WASM2C=$(WABT_ROOT)/bin/wasm2c

WASM2C_RUNTIME_PATH=$(WABT_ROOT)/wasm2c/
WASM2C_RUNTIME_FILES=$(addprefix $(WASM2C_RUNTIME_PATH), wasm-rt-impl.c uvwasi-rt.c)

UVWASI_PATH=$(WABT_ROOT)/third_party/uvwasi

DEPS=-I$(UVWASI_PATH)/include -L$(WABT_ROOT)/build/_deps/libuv-build -L$(WABT_ROOT)/build/third_party/uvwasi

LIBS=-luvwasi_a -luv_a -lpthread -ldl -lm

ALL_RESULTS=hello.wasm hello.wasm.c hello.elf

all: $(ALL_RESULTS)

clean:
	rm -rf $(ALL_RESULTS) hello.wasm.h *.o

hello.wasm: hello.c
	$(WASI_CLANG) --sysroot $(WASI_SYSROOT) $(WASM_CFLAGS) hello.c -o hello.wasm

hello.wasm.c: hello.wasm
	$(WASM2C) hello.wasm -o hello.wasm.c

hello.elf: hello.wasm.c  uvwasi-rt-main.c $(WASM2C_RUNTIME_FILES)
	$(CC) -g hello.wasm.c uvwasi-rt-main.c -o hello.elf -I$(WASM2C_RUNTIME_PATH) $(WASM2C_RUNTIME_FILES) $(DEPS) $(LIBS)

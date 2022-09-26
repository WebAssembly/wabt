WABT_ROOT=../../..
WASM2C=$(WABT_ROOT)/bin/wasm2c

WASM2C_RUNTIME_PATH=$(WABT_ROOT)/wasm2c/
WASM2C_RUNTIME_FILES=$(addprefix $(WASM2C_RUNTIME_PATH), wasm-rt-impl.c uvwasi-rt.c)

UVWASI_PATH=$(WABT_ROOT)/third_party/uvwasi

DEPS=-I$(UVWASI_PATH)/include -L$(WABT_ROOT)/build/_deps/libuv-build -L$(WABT_ROOT)/build/third_party/uvwasi

LIBS=-luvwasi_a -luv_a

DBG_FLAGS=-g
REL_FLAGS=-O3

CFLAGS=$(REL_FLAGS)

all: main.elf

clean:
	rm -rf $(ALL_RESULTS) main.* *.elf

main.c:
	$(WASM2C) main.wasm -o main.c

main.elf: main.c wasi-main.c $(WASM2C_RUNTIME_FILES)
	$(CC) $(CFLAGS) main.c wasi-main.c -o main.elf -I$(WASM2C_RUNTIME_PATH) $(WASM2C_RUNTIME_FILES) $(DEPS) $(LIBS)

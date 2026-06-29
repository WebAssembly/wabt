#include <stdio.h>
#include <stdlib.h>

#include "hello-wasi-wasm.h"
#include "wasm-rt-uvwasi-adapter.h"
#include "uvwasi.h"

int main(int argc, const char** argv) {
  w2c_hello__wasi hello = {0};
  struct w2c_wasi__snapshot__preview1 wasi = {0};
  uvwasi_t uvwasi = {0};
  uvwasi_options_t init_options = {0};
  uvwasi_preopen_t preopens[2] = {0};

  init_options.in = 0;
  init_options.out = 1;
  init_options.err = 2;
  init_options.fd_table_size = 10;

  extern const char** environ;
  init_options.argc = argc;
  init_options.argv = argv;
  init_options.envp = (const char**)environ;

  preopens[0].mapped_path = "/tmp";
  preopens[0].real_path = "/tmp";
  preopens[1].mapped_path = ".";
  preopens[1].real_path = ".";
  init_options.preopenc = 2;
  init_options.preopens = preopens;
  init_options.allocator = NULL;

  wasm_rt_init();
  uvwasi_errno_t ret = uvwasi_init(&uvwasi, &init_options);
  if (ret != UVWASI_ESUCCESS) {
    fprintf(stderr, "uvwasi_init failed with error %u\n", ret);
    return 1;
  }

  wasm2c_uvwasi_link(&wasi, &hello.w2c_memory, &uvwasi);
  wasm2c_hello__wasi_instantiate(&hello, &wasi);
  w2c_hello__wasi_0x5Fstart(&hello);
  wasm2c_hello__wasi_free(&hello);

  uvwasi_destroy(&uvwasi);
  wasm_rt_free();

  return 0;
}

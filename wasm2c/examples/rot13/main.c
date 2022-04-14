/* Entry point for the rot13 example.
 *
 * This example shows how you can fulfill wasm module imports in your C
 * program, and access linear memory.
 *
 * The program reads arguments from the command line, and [rot13] encodes them,
 * e.g.:
 *
 * ```
 * $ rot13 foo bar
 * foo -> sbb
 * bar -> one
 * ```
 *
 * [rot13]: https://en.wikipedia.org/wiki/ROT13
 */
#include <stdio.h>
#include <stdlib.h>

/* Uncomment this to define rot13_init rot13_Z_rot13 instead. */
/* #define WASM_RT_MODULE_PREFIX rot13_ */

#include "rot13.h"

/* Define the imports as declared in rot13.h. */
wasm_rt_memory_t (*Z_hostZ_mem);
u32 (*Z_hostZ_fill_buf)(u32, u32);
void (*Z_hostZ_buf_done)(u32, u32);

/* Define the implementations of the imports. */
static wasm_rt_memory_t s_memory;
static u32 fill_buf(u32 ptr, u32 size);
static void buf_done(u32 ptr, u32 size);

/* The string that is currently being processed. This needs to be static
 * because the buffer is filled in the callback. */
static const char* s_input;

int main(int argc, char** argv) {
  /* Initialize the Wasm runtime. */
  wasm_rt_init();

  /* Initialize the rot13 module. Since we didn't define WASM_RT_MODULE_PREFIX,
  the initialization function is called `init`. */
  init();

  /* Allocate 1 page of wasm memory (64KiB). */
  wasm_rt_allocate_memory(&s_memory, 1, 1);

  /* Provide the imports expected by the module: "host.mem", "host.fill_buf"
   * and "host.buf_done". Their mangled names are `Z_hostZ_mem`,
   * `Z_hostZ_fill_buf` and `Z_hostZ_buf_done`. */
  Z_hostZ_mem = &s_memory;
  Z_hostZ_fill_buf = &fill_buf;
  Z_hostZ_buf_done = &buf_done;

  /* Call `rot13` on each argument, using the mangled name. */
  while (argc > 1) {
    /* Move to next arg. Do this first, so the program name is skipped. */
    argc--; argv++;

    s_input = argv[0];
    Z_rot13();
  }

  /* Free the Wasm runtime state. */
  wasm_rt_free();

  return 0;
}

/* Fill the wasm buffer with the input to be rot13'd.
 *
 * params:
 *   ptr: The wasm memory address of the buffer to fill data.
 *   size: The size of the buffer in wasm memory.
 * result:
 *   The number of bytes filled into the buffer. (Must be <= size).
 */
u32 fill_buf(u32 ptr, u32 size) {
  for (size_t i = 0; i < size; ++i) {
    if (s_input[i] == 0) {
      return i;
    }
    s_memory.data[ptr + i] = s_input[i];
  }
  return size;
}

/* Called when the wasm buffer has been rot13'd.
 *
 * params:
 *   ptr: The wasm memory address of the buffer.
 *   size: The size of the buffer in wasm memory.
 */
void buf_done(u32 ptr, u32 size) {
  /* The output buffer is not necessarily null-terminated, so use the %*.s
   * printf format to limit the number of characters printed. */
  printf("%s -> %.*s\n", s_input, (int)size, &s_memory.data[ptr]);
}

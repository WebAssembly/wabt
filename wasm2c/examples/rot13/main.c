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

#include "rot13.h"

/* Define structure to hold the imports */
struct Z_host_instance_t {
  wasm_rt_memory_t memory;
  char* input;
};

/* Accessor to access the memory member of the host */
wasm_rt_memory_t* Z_hostZ_mem(struct Z_host_instance_t* instance) {
  return &instance->memory;
}

/* Declare the implementations of the imports. */
static u32 fill_buf(struct Z_host_instance_t* instance, u32 ptr, u32 size);
static void buf_done(struct Z_host_instance_t* instance, u32 ptr, u32 size);

/* Define host-provided functions under the names imported by the `rot13`
 * instance */
u32 Z_hostZ_fill_buf(struct Z_host_instance_t* instance, u32 ptr, u32 size) {
  return fill_buf(instance, ptr, size);
}

void Z_hostZ_buf_done(struct Z_host_instance_t* instance, u32 ptr, u32 size) {
  return buf_done(instance, ptr, size);
}

int main(int argc, char** argv) {
  /* Initialize the Wasm runtime. */
  wasm_rt_init();

  /* Initialize the rot13 module. */
  Z_rot13_init_module();

  /* Declare an instance of the `rot13` module. */
  Z_rot13_instance_t rot13_instance;

  /* Create a `host` module instance to store the memory and current string */
  struct Z_host_instance_t host_instance;
  /* Allocate 1 page of wasm memory (64KiB). */
  wasm_rt_allocate_memory(&host_instance.memory, 1, 1);

  /* Construct the module instance */
  Z_rot13_instantiate(&rot13_instance, &host_instance);

  /* Call `rot13` on each argument. */
  while (argc > 1) {
    /* Move to next arg. Do this first, so the program name is skipped. */
    argc--;
    argv++;

    host_instance.input = argv[0];
    Z_rot13Z_rot13(&rot13_instance);
  }

  /* Free the rot13 module. */
  Z_rot13_free(&rot13_instance);

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
u32 fill_buf(struct Z_host_instance_t* instance, u32 ptr, u32 size) {
  for (size_t i = 0; i < size; ++i) {
    if (instance->input[i] == 0) {
      return i;
    }
    instance->memory.data[ptr + i] = instance->input[i];
  }
  return size;
}

/* Called when the wasm buffer has been rot13'd.
 *
 * params:
 *   ptr: The wasm memory address of the buffer.
 *   size: The size of the buffer in wasm memory.
 */
void buf_done(struct Z_host_instance_t* instance, u32 ptr, u32 size) {
  /* The output buffer is not necessarily null-terminated, so use the %*.s
   * printf format to limit the number of characters printed. */
  printf("%s -> %.*s\n", instance->input, (int)size,
         &instance->memory.data[ptr]);
}

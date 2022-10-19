#include <stdio.h>
#include <stdlib.h>

#include "basic.wasm2c.h"


int main(int argc, char** argv) {
  /* Make sure there is at least one command-line argument. */
  if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);

    return 1;
  }

  s32 x = atoi(argv[1]);

  s32 in_a[4] = {x, x, x, x}; 
  s32 in_b[4] = {2*x, 2*x, 2*x, 2*x}; 
  int size = 4;

  printf("\t in_a = %d %d %d %d\n", in_a[0], in_a[1], in_a[2], in_a[3]);
  printf("\t in_b = %d %d %d %d\n", in_b[0], in_b[1], in_b[2], in_b[3]);

  // call WASM sandbox
  printf("-----------------------------\n");
  printf("Running WASM sandboxed SIMD\n");
  /* Initialize the Wasm runtime. */
  wasm_rt_init();

  /* One time initializations of minimum wasi runtime supported by wasm2c */
  Z_basic_init_module();
  
  /* Declare an instance of the `basic` module. */
  Z_basic_instance_t basic_instance;

  /* Construct the module instance */
  /* NOTE: the rot13 example passes in host_instance, where we store our values*/
  Z_basic_instantiate(&basic_instance);

  // load 4 32-bit numbers
  for (int i = 0; i < size; i++) {
    basic_instance.w2c_memory.data[4*i] = in_a[i]; 
    basic_instance.w2c_memory.data[4*(i+size)] = in_b[i]; 
  }
  
  printf("- SIMD Add then sum result\n");

  int total = Z_basicZ_total_sum(&basic_instance, 0, size);
  printf("Total: %d \n", total);

  /* Free the basic module. */
  Z_basic_free(&basic_instance);

  /* Free the Wasm runtime state. */
  wasm_rt_free();

  return 0;
}
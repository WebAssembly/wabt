#include <stdio.h>
#include <stdlib.h>

/* Uncomment this to define fac_init and fac_Z_facZ_ii instead. */
/* #define WASM_RT_MODULE_PREFIX fac_ */

#include "fac.h"

int main(int argc, char** argv) {
  /* Make sure there is at least one command-line argument. */
  if (argc < 2) return 1;

  /* Convert the argument from a string to an int. We'll implictly cast the int
  to a `u32`, which is what `fac` expects. */
  u32 x = atoi(argv[1]);

  /* Retrieve sandbox details */
  wasm2c_sandbox_funcs_t sbx_details = get_wasm2c_sandbox_info();

  /* One time initializations of minimum wasi runtime supported by wasm2c */
  sbx_details.wasm_rt_sys_init();

  /* Optional upper limit for number of wasm pages for this module.
  0 means no limit  */
  int max_wasm_page = 0;

  /* Create a sandbox instance */
  wasm2c_sandbox_t* sbx_instance = (wasm2c_sandbox_t*) sbx_details.create_wasm2c_sandbox(max_wasm_page);

  /* Call `fac`, using the mangled name. */
  u32 result = w2c_fac(sbx_instance, x);

  /* Print the result. */
  printf("fac(%u) -> %u\n", x, result);

  /* Destroy the sandbox instance */
  sbx_details.destroy_wasm2c_sandbox(sbx_instance);

  return 0;
}

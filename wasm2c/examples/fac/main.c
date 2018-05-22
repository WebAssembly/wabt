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

  /* Initialize the fac module. Since we didn't define WASM_RT_MODULE_PREFIX,
  the initialization function is called `init`. */
  init();

  /* Call `fac`, using the mangled name. */
  u32 result = Z_facZ_ii(x);

  /* Print the result. */
  printf("fac(%u) -> %u\n", x, result);

  return 0;
}

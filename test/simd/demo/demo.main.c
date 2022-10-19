#include <stdio.h>
#include <stdlib.h>

#include "demo.wasm2c.h"

int main(int argc, char** argv) {
  /* Make sure there is at least one command-line argument. */
  if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
  }

  s32 x = atoi(argv[1]);

  // Run it locally without the sandbox
  s32 in_a[4] = {x, x, x, x}; 
  s32 in_a_neg[4] = {-x, -x, -x, -x}; 
  s32 in_a_alt[4] = {x, -x, x, -x}; 
  s32 in_b[4] = {2*x, 2*x, 2*x, 2*x}; 
  int size = 4;

  // call WASM sandbox
  printf("-----------------------------\n");
  printf("Running WASM sandboxed SIMD\n");

  /* Initialize the Wasm runtime. */
  wasm_rt_init();

    /* One time initializations of minimum wasi runtime supported by wasm2c */
  Z_demo_init_module();
  
  Z_demo_instance_t demo_instance;
  Z_demo_instantiate(&demo_instance);

  // this is the pointer in w2c_memory
  s32 sbx_in_a = 0;
  s32 sbx_in_b = 4;
  s32 out = 8;


  for (int i = 0; i < size; i++) {
    demo_instance.w2c_memory.data[(sbx_in_a + i)*4] = in_a[i];
    demo_instance.w2c_memory.data[(sbx_in_b + i)*4] = in_b[i];
  }
  
  Z_demoZ_all_the_f32x4(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the f32x4 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_f64x2(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the f64x2 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_i8x16(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the i8x16 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_u8x16(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the u8x16 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_i16x8(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the i16x8 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_u16x8(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the u16x8 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_i32x4(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the i32x4 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_u32x4(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the u32x4 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );


  Z_demoZ_all_the_i64x2(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the i64x2 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_u64x2(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the u64x2 result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  Z_demoZ_all_the_v128(&demo_instance, sbx_in_a, sbx_in_b, out);

  printf("All the v128 Result: %d %d %d %d\n", 
    demo_instance.w2c_memory.data[(out + 0)*4],
    demo_instance.w2c_memory.data[(out + 1)*4],
    demo_instance.w2c_memory.data[(out + 2)*4],
    demo_instance.w2c_memory.data[(out + 3)*4]
  );

  /* Free the Wasm runtime state. */
  wasm_rt_free();

  return 0;
}
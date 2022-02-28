# wasm2c: Convert wasm files to C source and header

`wasm2c` takes a WebAssembly module and produces an equivalent C source and
header. Some examples:

```sh
# parse binary file test.wasm and write test.c and test.h
$ wasm2c test.wasm -o test.c

# parse test.wasm, write test.c and test.h, but ignore the debug names, if any
$ wasm2c test.wasm --no-debug-names -o test.c
```

## Tutorial: .wat -> .wasm -> .c

Let's look at a simple example of a factorial function.

```wasm
(memory $mem 1)
(func (export "fac") (param $x i32) (result i32)
  (if (result i32) (i32.eq (local.get $x) (i32.const 0))
    (then (i32.const 1))
    (else
      (i32.mul (local.get $x) (call 0 (i32.sub (local.get $x) (i32.const 1))))
    )
  )
)
```

Save this to `fac.wat`. We can convert this to a `.wasm` file by using the
`wat2wasm` tool:

```sh
$ wat2wasm fac.wat -o fac.wasm
```

We can then convert it to a C source and header by using the `wasm2c` tool:

```sh
$ wasm2c fac.wasm -o fac.c
```

This generates two files, `fac.c` and `fac.h`. We'll take a closer look at
these files below, but first let's show a simple example of how to use these
files.

## Using the generated module

To actually use our fac module, we'll use create a new file, `main.c`, that
include `fac.h`, initializes the module, and calls `fac`.

`wasm2c` generates a few symbols for us, `init` and `Z_facZ_ii`. `init`
initializes the module, and `Z_facZ_ii` is our exported `fac` function, but
[name-mangled](https://en.wikipedia.org/wiki/Name_mangling) to include the
function signature.

In addition to parameters defined in `fac.wat`, `init` and `Z_facZ_ii` also
takes in a pointer to `Z_fac_module_instance_t` (name-mangled `instance_module_instance`). 
The structure is used to store the context information of the current intance
and `main.c` is responsible for providing it.

We can define `WASM_RT_MODULE_PREFIX` before including `fac.h` to generate
these symbols with a prefix, in case we already have a symbol called `init` (or
even `Z_facZ_ii`!) Note that you'll have to compile `fac.c` with this macro
too, for this to work.

```c
#include <stdio.h>
#include <stdlib.h>

/* Uncomment this to define fac_init and fac_Z_facZ_ii instead. */
/* #define WASM_RT_MODULE_PREFIX fac_ */

#include "fac.h"

int main(int argc, char** argv) {
  /* Make sure there is at least one command-line argument. */
  if (argc < 2)
    return 1;

  /* Convert the argument from a string to an int. We'll implicitly cast the int
  to a `u32`, which is what `fac` expects. */
  u32 x = atoi(argv[1]);

  /* Initialize the Wasm runtime. */
  wasm_rt_init();

  /* Declare and initialize the structure for context info. */
  Z_fac_module_instance_t module_instance;

  /* Initialize the fac module. Since we didn't define WASM_RT_MODULE_PREFIX,
  the initialization function is called `init`. */
  init(&module_instance);

  /* Call `fac`, using the mangled name. */
  u32 result = Z_facZ_ii(&module_instance, x);

  /* Print the result. */
  printf("fac(%u) -> %u\n", x, result);

  /* Free the Wasm runtime state. */
  wasm_rt_free();

  return 0;
}
```

To compile the executable, we need to use `main.c` and the generated `fac.c`.
We'll also include `wasm-rt-impl.c` which has implementations of the various
`wasm_rt_*` functions used by `fac.c` and `fac.h`.

```sh
$ cc -o fac main.c fac.c wasm-rt-impl.c
```

Now let's test it out!

```sh
$ ./fac 1
fac(1) -> 1
$ ./fac 5
fac(5) -> 120
$ ./fac 10
fac(10) -> 3628800
```

You can take a look at the all of these files in
[wasm2c/examples/fac](/wasm2c/examples/fac).

## Looking at the generated header, `fac.h`

The generated header file looks something like this:

```c
#ifndef FAC_H_GENERATED_
#define FAC_H_GENERATED_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "wasm-rt.h"

#ifndef WASM_RT_MODULE_PREFIX
#define WASM_RT_MODULE_PREFIX
#endif

#define WASM_RT_PASTE_(x, y) x ## y
#define WASM_RT_PASTE(x, y) WASM_RT_PASTE_(x, y)
#define WASM_RT_ADD_PREFIX(x) WASM_RT_PASTE(WASM_RT_MODULE_PREFIX, x)

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;

typedef struct {
  wasm_rt_memory_t w2c_M0;
} Z_fac_module_instance_t;

extern void WASM_RT_ADD_PREFIX(init)(Z_fac_module_instance_t *);

/* export: 'fac' */
extern u32 (*WASM_RT_ADD_PREFIX(Z_facZ_ii))(Z_fac_module_instance_t *, u32);
#ifdef __cplusplus
}
#endif

#endif /* FAC_H_GENERATED_ */
```

Let's look at each section. The outer `#ifndef` is standard C boilerplate for a
header. The `extern "C"` part makes sure to not mangle the symbols if using
this header in C++.

```c
#ifndef FAC_H_GENERATED_
#define FAC_H_GENERATED_
#ifdef __cplusplus
extern "C" {
#endif
  ...

#ifdef __cplusplus
}
#endif
#endif /* FAC_H_GENERATED_ */
```

`wasm-rt.h` contains a number of definitions required for
all WebAssembly modules, which will be explained in the following section.

```c
#include "wasm-rt.h"
```

Next we can specify a module prefix. This is useful if you are using multiple
modules that may use the same name as an export. Since we only have one module
here, it's fine to use the default which is an empty prefix:

```c
#ifndef WASM_RT_MODULE_PREFIX
#define WASM_RT_MODULE_PREFIX
#endif

#define WASM_RT_PASTE_(x, y) x##y
#define WASM_RT_PASTE(x, y) WASM_RT_PASTE_(x, y)
#define WASM_RT_ADD_PREFIX(x) WASM_RT_PASTE(WASM_RT_MODULE_PREFIX, x)
```

Next are some convenient typedefs for integers and floats of fixed sizes:

```c
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;
```

Next is the definition of `Z_fac_module_instance_t`, which contains the memory region
in `fac.wat`.

```c
typedef struct {
  wasm_rt_memory_t w2c_M0;
} Z_fac_module_instance_t;
```

## Looking at the wasm-rt header, `wasm-rt.h`
`wasm-rt.h` contains a number of definitions required for all WebAssembly modules.

First is the `wasm_rt_trap_t` enum, which is used to give the reason a trap
occurred.

```c
typedef enum {
  WASM_RT_TRAP_NONE,
  WASM_RT_TRAP_OOB,
  WASM_RT_TRAP_INT_OVERFLOW,
  WASM_RT_TRAP_DIV_BY_ZERO,
  WASM_RT_TRAP_INVALID_CONVERSION,
  WASM_RT_TRAP_UNREACHABLE,
  WASM_RT_TRAP_CALL_INDIRECT,
  WASM_RT_TRAP_EXHAUSTION,
} wasm_rt_trap_t;
```

Next is the `wasm_rt_type_t` enum, which is used for specifying function
signatures. The four WebAssembly value types are included:

```c
typedef enum {
  WASM_RT_I32,
  WASM_RT_I64,
  WASM_RT_F32,
  WASM_RT_F64,
} wasm_rt_type_t;
```

Next is `wasm_rt_funcref_t`, the function signature for a generic function
callback. Since a WebAssembly table can contain functions of any given
signature, it is necessary to convert them to a canonical form:

```c
typedef void (*wasm_rt_funcref_t)(void);
```

Next are the definitions for a table element. `func_type` is a function index
as returned by `wasm_rt_register_func_type` described below.

```c
typedef struct {
  uint32_t func_type;
  wasm_rt_funcref_t func;
} wasm_rt_elem_t;
```

Next is the definition of a memory instance. The `data` field is a pointer to
`size` bytes of linear memory. The `size` field of `wasm_rt_memory_t` is the
current size of the memory instance in bytes, whereas `pages` is the current
size in pages (65536 bytes.) `max_pages` is the maximum number of pages as
specified by the module, or `0xffffffff` if there is no limit.

```c
typedef struct {
  uint8_t* data;
  uint32_t pages, max_pages;
  uint32_t size;
} wasm_rt_memory_t;
```

Next is the definition of a table instance. The `data` field is a pointer to
`size` elements. Like a memory instance, `size` is the current size of a table,
and `max_size` is the maximum size of the table, or `0xffffffff` if there is no
limit.

```c
typedef struct {
  wasm_rt_elem_t* data;
  uint32_t max_size;
  uint32_t size;
} wasm_rt_table_t;
```

## Symbols that must be defined by the embedder
Next in `wasm-rt.h` are a collection of extern symbols that must be implemented by
the embedder (i.e. you) before this C source can be used.

A C implementation of these functions is defined in
[`wasm-rt-impl.h`](wasm-rt-impl.h) and [`wasm-rt-impl.c`](wasm-rt-impl.c).

```c
extern void wasm_rt_trap(wasm_rt_trap_t) __attribute__((noreturn));
extern uint32_t wasm_rt_register_func_type(uint32_t params, uint32_t results, ...);
extern void wasm_rt_allocate_memory(wasm_rt_memory_t*, uint32_t initial_pages, uint32_t max_pages);
extern uint32_t wasm_rt_grow_memory(wasm_rt_memory_t*, uint32_t pages);
extern void wasm_rt_allocate_table(wasm_rt_table_t*, uint32_t elements, uint32_t max_elements);
```

`wasm_rt_trap` is a function that is called when the module traps. Some
possible implementations are to throw a C++ exception, or to just abort the
program execution.

`wasm_rt_register_func_type` is a function that registers a function type. It
is a variadic function where the first two arguments give the number of
parameters and results, and the following arguments are the types. For example,
the function `func (param i32 f32) (result f64)` would register the function
type as
`wasm_rt_register_func_type(2, 1, WASM_RT_I32, WASM_RT_F32, WASM_RT_F64)`.

`wasm_rt_allocate_memory` initializes a memory instance, and allocates at least
enough space for the given number of initial pages. The memory must be cleared
to zero.

`wasm_rt_grow_memory` must grow the given memory instance by the given number
of pages. If there isn't enough memory to do so, or the new page count would be
greater than the maximum page count, the function must fail by returning
`0xffffffff`. If the function succeeds, it must return the previous size of the
memory instance, in pages.

`wasm_rt_allocate_table` initializes a table instance, and allocates at least
enough space for the given number of initial elements. The elements must be
cleared to zero.

## Exported symbols

Finally, `fac.h` defines exported symbols provided by the module. In our
example, the only function we exported was `fac`. An additional function is
provided called `init`, which initializes the module and must be called before
the module can be used:

```c
extern void WASM_RT_ADD_PREFIX(init)(Z_fac_module_instance_t *);

/* export: 'fac' */
extern u32 (*WASM_RT_ADD_PREFIX(Z_facZ_ii))(Z_fac_module_instance_t *, u32);
```

All exported names use `WASM_RT_ADD_PREFIX` (as described above) to allow the
symbols to placed in a namespace as decided by the embedder. All symbols are
also mangled so they include the types of the function signature.

In our example, `Z_facZ_ii` is the mangling for a function named `fac` that
takes one `i32` parameter and returns one `i32` result. `Z_facZ_ii` is initialized
in `fac.c` as:

```c
/* export: 'fac' */
u32 (*WASM_RT_ADD_PREFIX(Z_facZ_ii))(Z_fac_module_instance_t *, u32) = (&w2c_fac);
```

## Handling other kinds of imports and exports of modules
Exported functions are handled through pointers to the function being exported. If
a module is trying to import some function, `wasm2c` declares a function pointer in
the output header file, and the host function is responsible for initializing the 
pointer before executing the program.

Exports of other kinds (globals, memories, tables) are handled differently, since
they are part of `module_instance_t`, and each instance should be able to have its
own exports. For these cases, `wasm2c` provides a function that takes in a module
instance as argument, and returns the corresponding exports. For example, if `fac`
exports its memory as such:

```wasm
(export "mem" (memory $mem))
```

and then `wasm2c` declares the following function in the header:

```c
/* export: 'mem' */
extern wasm_rt_memory_t* WASM_RT_ADD_PREFIX(Z_mem)(Z_fac_module_instance_t *);
```

which is defined as:
```c
/* export: 'mem' */
wasm_rt_memory_t* WASM_RT_ADD_PREFIX(Z_mem)(Z_fac_module_instance_t *module_instance) {
  return &module_instance->w2c_M0;
}
```

## A quick look at `fac.c`

The contents of `fac.c` are internals, but it is useful to see a little about
how it works.

The first few hundred lines define macros that are used to implement the
various WebAssembly instructions. Their implementations may be interesting to
the curious reader, but are out of scope for this document.

Following those definitions are various initialization functions (`init`,
`init_func_types`, `init_globals`, `init_memory`, and `init_table`.) In our 
example, most of these functions are empty, since the module doesn't use any 
globals or tables.

The most interesting part is the definition of the function `fac`:

```c
static u32 fac(instance_stack_info *, instance_memories *, u32 p0) {
  FUNC_PROLOGUE;
  u32 i0, i1, i2;
  i0 = p0;
  i1 = 0u;
  i0 = i0 == i1;
  if (i0) {
    i0 = 1u;
  } else {
    i0 = p0;
    i1 = p0;
    i2 = 1u;
    i1 -= i2;
    i1 = fac(i1);
    i0 *= i1;
  }
  FUNC_EPILOGUE;
  return i0;
}
```

If you look at the original WebAssembly text in the flat format, you can see
that there is a 1-1 mapping in the output:

```wasm
(func $fac (param $x i32) (result i32)
  local.get $x
  i32.const 0
  i32.eq
  if (result i32)
    i32.const 1
  else
    local.get $x
    local.get $x
    i32.const 1
    i32.sub
    call 0
    i32.mul
  end)
```

This looks different than the factorial function above because it is using the
"flat format" instead of the "folded format". You can use `wat-desugar` to
convert between the two to be sure:

```sh
$ wat-desugar fac-flat.wat --fold -o fac-folded.wat
```

```wasm
(module
  (func (;0;) (param i32) (result i32)
    (if (result i32)  ;; label = @1
      (i32.eq
        (local.get 0)
        (i32.const 0))
      (then
        (i32.const 1))
      (else
        (i32.mul
          (local.get 0)
          (call 0
            (i32.sub
              (local.get 0)
              (i32.const 1)))))))
  (export "fac" (func 0))
  (type (;0;) (func (param i32) (result i32))))
```

The formatting is different and the variable and function names are gone, but
the structure is the same.

## Instantiate multiple instances of a module
Since information about the execution context, such as memories, is encapsulated
in `instance_module_instance`, and a pointer to the structures is being passed through 
function calls, multiple instances of the same module can be instantiated 
with different context. 

We can take a look at another version of the `main` function for `rot13` example. By 
declaring two sets of context information, two instances of `rot13` can be instantiated 
in the same address space.

```c
#include "rot13.h"

/* Define the imports as declared in rot13.h. */
u32 (*Z_hostZ_fill_bufZ_iii)(Z_rot13_module_instance_t *, u32, u32);
void (*Z_hostZ_buf_doneZ_vii)(Z_rot13_module_instance_t *, u32, u32);

/* Define the implementations of the imports. */
static u32 fill_buf(Z_rot13_module_instance_t * module_instance, u32 ptr, u32 size);
static void buf_done(Z_rot13_module_instance_t * module_instance, u32 ptr, u32 size);

/* The string that is currently being processed. This needs to be static
 * because the buffer is filled in the callback. */
static const char* s_input;

int main(int argc, char** argv) {
  /* Define two sets of context info */
  Z_rot13_module_instance_t module_instance_one;
  Z_rot13_module_instance_t module_instance_two;

  /* Define the imported memories */
  wasm_rt_memory_t s_memory_one;
  wasm_rt_memory_t s_memory_two;
  module_instance_one.Z_hostZ_mem = &s_memory_one;
  module_instance_two.Z_hostZ_mem = &s_memory_two;

  /* Initialize the rot13 module. Since we didn't define WASM_RT_MODULE_PREFIX,
  the initialization function is called `init`. */
  init(&module_instance_one);
  init(&module_instance_two);

  /* Allocate 1 page of wasm memory (64KiB). */
  wasm_rt_allocate_memory(&s_memory_one, 1, 1);
  wasm_rt_allocate_memory(&s_memory_two, 1, 1);

  /* Provide the imports expected by the module: "host.mem", "host.fill_buf"
   * and "host.buf_done". Their mangled names are `Z_hostZ_mem`,
   * `Z_hostZ_fill_bufZ_iii` and `Z_hostZ_buf_doneZ_vii`. */
  Z_hostZ_fill_bufZ_iii = &fill_buf;
  Z_hostZ_buf_doneZ_vii = &buf_done;

  /* Call `rot13` on first two argument, using the mangled name. */
  assert(argc > 2);
  s_input = argv[0];
  Z_rot13Z_vv(&module_instance_one);
  s_input = argv[1];
  Z_rot13Z_vv(&module_instance_two);

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
u32 fill_buf(Z_rot13_module_instance_t *module_instance, u32 ptr, u32 size) {
  for (size_t i = 0; i < size; ++i) {
    if (s_input[i] == 0) {
      return i;
    }
    module_instance->Z_hostZ_mem->data[ptr + i] = s_input[i];
  }
  return size;
}

/* Called when the wasm buffer has been rot13'd.
 *
 * params:
 *   ptr: The wasm memory address of the buffer.
 *   size: The size of the buffer in wasm memory.
 */
void buf_done(Z_rot13_module_instance_t *module_instance, u32 ptr, u32 size) {
  /* The output buffer is not necessarily null-terminated, so use the %*.s
   * printf format to limit the number of characters printed. */
  printf("%s -> %.*s\n", s_input, (int)size, &module_instance->Z_hostZ_mem->data[ptr]);
}
```

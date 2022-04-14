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

To actually use our `fac` module, we'll use create a new file, `main.c`, that
include `fac.h`, initializes the module, and calls `fac`.

`wasm2c` generates a few C symbols based on the `fac.wasm` module.  `Z_fac_init`
and `Z_fac_Z_fac`.  The former initializes the module, and the later is our
exported `fac` function.

All the exported symbols shared a common prefix (`Z_fac`) which, by default, is
based on the name section in the module or the name of input file.  This prefix
can be overridden using the `-n/--module-name` command line flag.

```c
#include <stdio.h>
#include <stdlib.h>

#include "fac.h"

int main(int argc, char** argv) {
  /* Make sure there is at least one command-line argument. */
  if (argc < 2) return 1;

  /* Convert the argument from a string to an int. We'll implicitly cast the int
  to a `u32`, which is what `fac` expects. */
  u32 x = atoi(argv[1]);

  /* Initialize the Wasm runtime. */
  wasm_rt_init();

  /* Initialize the fac module. */
  Z_fac_init();

  /* Call `fac`, using the mangled name. */
  u32 result = Z_facZ_fac(x);

  /* Print the result. */
  printf("fac(%u) -> %u\n", x, result);

  /* Free the fac module. */
  Z_fac_free();

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

...

#include "wasm-rt.h"

...

#ifndef WASM_RT_CORE_TYPES_DEFINED
#define WASM_RT_CORE_TYPES_DEFINED
...
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void Z_fac_init(void);
extern void Z_fac_free(void);

/* export: 'fac' */
extern u32 (*Z_facZ_fac)(u32);

#ifdef __cplusplus
}
#endif

#endif  /* FAC_H_GENERATED_ */
```

Let's look at each section. The outer `#ifndef` is standard C
boilerplate for a header. This `WASM_RT_CORE_TYPES_DEFINED` section
contains a number of definitions required for all WebAssembly
modules. The `extern "C"` part makes sure to not mangle the symbols if
using this header in C++.

The included `wasm-rt.h` file also includes a number of relevant definitions.
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
void wasm_rt_init(void);
void wasm_rt_free(void);
extern void wasm_rt_trap(wasm_rt_trap_t) __attribute__((noreturn));
const char* wasm_rt_strerror(wasm_rt_trap_t trap);
extern uint32_t wasm_rt_register_func_type(uint32_t params, uint32_t results, ...);
extern void wasm_rt_allocate_memory(wasm_rt_memory_t*, uint32_t initial_pages, uint32_t max_pages);
extern uint32_t wasm_rt_grow_memory(wasm_rt_memory_t*, uint32_t pages);
extern void wasm_rt_free_memory(wasm_rt_memory_t*);
extern void wasm_rt_allocate_table(wasm_rt_table_t*, uint32_t elements, uint32_t max_elements);
extern void wasm_rt_free_table(wasm_rt_table_t*);
extern uint32_t wasm_rt_call_stack_depth; /* on platforms that don't use the signal handler to detect exhaustion */
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

`wasm_rt_free_memory` frees the memory instance.

`wasm_rt_allocate_table` initializes a table instance, and allocates at least
enough space for the given number of initial elements. The elements must be
cleared to zero.

`wasm_rt_free_table` frees the table instance.

`wasm_rt_call_stack_depth` is the current stack call depth. Since this is
shared between modules, it must be defined only once, by the embedder.
It is only used on platforms that don't use the signal handler to detect
exhaustion.

## Exported symbols

Finally, `fac.h` defines exported symbols provided by the module. In our
example, the only function we exported was `fac`. Two additional
functions are provided called `init`, which initializes the module and
must be called before the module can be used, and `free`, which frees
the module's state (its memory and table instances).

```c
extern void Z_fac_init(void);
extern void Z_fac_free(void);

/* export: 'fac' */
extern u32 (*Z_facZ_fac)(u32);
```

All exported names use the mangled module name as a prefix (as described above)
to avoid collisions with other C symbols or symbols from other modules.

In our example, `Z_facZ_fac` is the mangling of the function named `fac`.

## A quick look at `fac.c`

The contents of `fac.c` are internals, but it is useful to see a little about
how it works.

The first few hundred lines define macros that are used to implement the
various WebAssembly instructions. Their implementations may be interesting to
the curious reader, but are out of scope for this document.

Following those definitions are various initialization functions (`init`, `free`,
`init_func_types`, `init_globals`, `init_memory`, `init_table`, and
`init_exports`.) In our example, most of these functions are empty, since the
module doesn't use any globals, memory or tables.

The most interesting part is the definition of the function `fac`:

```c
static u32 w2c_fac(u32 w2c_p0) {
  FUNC_PROLOGUE;
  u32 w2c_i0, w2c_i1, w2c_i2;
  w2c_i0 = w2c_p0;
  w2c_i1 = 0u;
  w2c_i0 = w2c_i0 == w2c_i1;
  if (w2c_i0) {
    w2c_i0 = 1u;
  } else {
    w2c_i0 = w2c_p0;
    w2c_i1 = w2c_p0;
    w2c_i2 = 1u;
    w2c_i1 -= w2c_i2;
    w2c_i1 = w2c_fac(w2c_i1);
    w2c_i0 *= w2c_i1;
  }
  FUNC_EPILOGUE;
  return w2c_i0;
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

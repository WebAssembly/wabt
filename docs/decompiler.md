# wasm-decompile

Decompiles binary wasm modules into a text format that is significantly
more compact and familiar (for users of C-style languages).

Example:

`bin/wasm-decompile test.wasm -o test.dcmp`

## Goals.

This tool is aimed at users that want to be able to "read" large volumes
of Wasm code such as language, runtime and tool developers, or any programmers
that may not have the source code of the generated wasm available, or are
trying to understand what the generated code does.

The syntax has been designed to be as light-weight and as readable as possible,
while still allowing one to see the underlying Wasm constructs clearly.

## Non-goals.

Be a programming language.

Though compiling this output code back into a wasm module is possible,
such functionality is currently not provided. The format is very low-level,
much like Wasm itself, so even though it looks more high level than the .wat
format, it wouldn't be any more suitable for general purpose programming.

## Language guide.

This section shows some aspects of the language in terms of how they map to
Wasm and/or how they might differ from a typical C-like language. It does
not try to define the actual semantics of Wasm, the reader is expected to
already be mostly familiar with that.

### Naming.

wasm-decompile, much like wasm2wat, derives names from import/export
declarations and the name section where possible. For things that have no
names, names are generated starting from `a`, `b`, `c` and so forth.

In addition, prefixes are used for things that are not arguments/locals:
`f_` for functions, `g_` for globals, etc.

Existing names may be generated "demangled" C++ function signatures, which
in the case of functions using STL types may end up several hundred characters
long. Besides removing characters not typically part of an identifier, the
decompiler also strips common keywords/types from these in an effort to
reduce their size.

### Top level declarations.

Top level items may be preceded with `import` or `export`.

Memory is declared like `memory m(initial: 1, max: 0);`

Globals: `global my_glob:int;`

Data: `data d_a(offset: 0) = "Hello, World!";`

Functions (see below for instructions that may appear between `{}`):
`function f(a:int, b:int):int { return a + b; }`

### Statements and expressions.

An expression is generated for any sequence of Wasm instructions that
leave exactly 1 value on the stack.

For instructions that leave no value values on the stack, a statement is
generated, which is an expression that sits on its own line in the context
of a control-flow block, or the function itself. A statement may also be
generated for expressions that return a value through control flow, such
as a branch instruction.

Instructions that leave multiple values on the stack, or otherwise do stack
operations that break the "expression order", instead force the values to be
written to temporary variables (named `t1`, `t2` etc) which the subsequent
instructions can then operate upon (this does not happen with MVP-only code).

### Declaration of arguments and locals.

Arguments are defined in the function signature, as shown above.

Locals are defined upon first use: `var my_local:int = 1;`

### Types.

The decompiler uses `int` and `long` for 32-bit and 64-bit integers, and
`float` and `double` for 32-bit and 64-bit floating point numbers.

Besides these, there are the types `byte` and `ubyte` (8-bit), `short` and
`ushort` (16-bit), and `uint`, which are used exclusively with certain
load/store operations.

### Loads and stores.

These tend to be the hardest to "read" in Wasm code, as they've lost all
context of the data structures and types the language that Wasm was compiled
from was operating upon.

wasm-decompile has a few feature to try and make these more readable.

The basic form looks like an array indexing operation, so `o[2]:int` says: read
element 2 from `o` when seen as an array of ints. This thus accesses 4 bytes
at byte-offset 8.

`o` is just declared as an `int`, since there is no such thing as a pointer
type in Wasm. But wasm-decompile tries to derive them. For example, if the
code is doing `o[0]:int = o[1]:int + o[2]:int`, then wasm-decompile assumes
`o` points to a struct with 3 ints, and may instead compile this to:

    var o:{ a:int, b:int, c:int };
    o.a = o.b + o.c

The `{}` type is a nameless struct declaration (named ones tbd) that hints the
reader at what kind of memory layout `o` is accessing. This seems more
informative than just uncorrelated indices all over the code.

Sadly, optimized output from a compiler like LLVM often reworks memory accesses
in such crazy ways that this "struct detection" fails, for example it falls
back to indexing operations when there are holes or overlaps in the memory
layout, or types are mixed, etc. This happens even more so when locals such
as `o` are being re-used for unrelated things in memory.

Additionally, wasm-decompile tried to clean up typical indexing operations.
For example, when accessing any array of 32-bit elements, generated Wasm
code often looks like `(base + (index << 2))[0]:int`, since Wasm has no
built-in way to scale the index by the type of thing being loaded.
wasm-decompile then transforms this into just `base[index]:int`, since the
scaling of anything between the `[]` by the type size is already implied.

### Control flow.

Wasm's if-then maps fairly directly to a C-like `if (c) { 1; } else { 2; }`.
Unlike most languages, these if-thens can also be expressions, as shown in
this example (wasm-decompile does not currently use the `?:` ternary).

Wasm's loop becomes a `loop L { ...; continue L; }` structure. The
inclusion of a label means nested loops can continue any of them.

Wasm's blocks are little more than a label for forward jumps, and cause
excessive amounts of nesting in other text formats such as .wat, so here
they are reduced to what they naturally are: a label. This label uses `{}`
for denoting a block only when used as an expression, so typically does not
indent, and thus doesn't cause endless nesting:

    if (c) goto L;
    ...
    label L:

### Operator precedence.

wasm-decompile uses the following operator precedence to reduce the amount of
`()` needed in expressions, from high (needs no `()`) to low (always needs
`()` when nested):

* `()`, `a`, `1`, `a()`
* `[]`
* `if () {} else {}`
* `*`, `/`, `%`
* `+`, `-`
* `<<`, `>>`
* `==`, `!=`, `<`, `>`, `>=`, `<=`
* `&`, `|`
* `min`, `max`
* `=`

Only `+` and `*` are associative, i.e. can have multiple of them in sequence
without additional `()`.

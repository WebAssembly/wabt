[![Build Status](https://travis-ci.org/WebAssembly/sexpr-wasm-prototype.svg?branch=master)](https://travis-ci.org/WebAssembly/sexpr-wasm-prototype)

# sexpr-wasm-prototype

A suite of build tools for WebAssembly, including:

 - **sexpr-wasm**: translate from [s-expressions](https://github.com/WebAssembly/spec) to the WebAssembly [binary-encoding](https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md)
 - **wasm-wast**: the inverse of sexpr-wasm, translate from the binary encoding back to an s-expression source file (also known as a .wast)
 - **wasm-interp**: decode and run a WebAssembly binary file using a stack-based interpreter

## Cloning

Clone as normal, but don't forget to update/init submodules as well:

```
$ git clone https://github.com/WebAssembly/sexpr-wasm-prototype
$ cd sexpr-wasm-prototype
$ git submodule update --init
```

This will fetch the v8, testsuite and gtest repos, which are needed for some
tests.

## Building

You'll need [CMake](https://cmake.org). If you just run `make`, it will run
CMake for you, and put the result in `out/clang/Debug/` by default:

```
$ make
```

This will build the default version of the tools: a debug build using the Clang
compiler. It will also create a symlinks to the built binaries in the `out/`
directory.

There are many make targets available for other configurations as well. They
are generated from every combination of a compiler, build type and
configuration.

 - compilers: `gcc`, `clang`, `gcc-i686`, `gcc-fuzz`
 - build types: `debug`, `release`
 - configurations: empty, `asan`, `msan`, `lsan`, `ubsan`, `no-re2c-bison`,
   `no-tests`

They are combined with dashes, for example:

```
$ make clang-debug
$ make gcc-i686-release
$ make clang-debug-lsan
$ make gcc-debug-no-re2c-bison
```

You can also run CMake yourself, the normal way:

```
$ mkdir build
$ cd build
$ cmake ..
...
```

If you make changes to `src/wasm-ast-parser.y`, you'll need to install Bison.
Before you upload your PR, please run `make update-bison` to update the
prebuilt C sources in `src/prebuilt/`.

If you make changes to `src/wasm-ast-lexer.c`, you'll need to install
[re2c](http://re2c.org). Before you upload your PR, please run `make
update-re2c` to update the prebuilt C sources in `src/prebuilt/`.

CMake will detect if you don't have re2c or Bison installed and use the
prebuilt source files instead.

## Building a JavaScript engine

The d8 and spidermonkey JavaScript engines can load and run the generated
binary-encoded files. Some of the tests rely on one of these executables. To
build it:

```
$ scripts/build-d8.sh
$ scripts/build-sm.sh
...
```

When the build script is finished, the d8 exectuable will be in the
`third_party/v8/v8/out/Release/d8` directory. The spidermonkey executable will
be in the `third_party/gecko-dev/js/src/build_OPT.OBJ/js/src` directory.

You can also download a prebuilt version (the same one used to test on Travis)
by running the `download-d8.sh` or `download-sm.sh` scripts:

```
$ scripts/download-d8.sh
$ scripts/download-sm.sh
...
```

This downloads the executable into the `out` directory. The test runner will
look here if there is no executable in the built directory.

## Running sexpr-wasm

Some examples:

```
# parse and typecheck test.wast
$ out/sexpr-wasm test.wast

# parse test.wast and write to binary file test.wasm
$ out/sexpr-wasm test.wast -o test.wasm

# parse spec-test.wast, and write verbose output to stdout (including the
# meaning of every byte)
$ out/sexpr-wasm spec-test.wast -v

# parse spec-test.wast, and write files to spec-test.json. Modules are written
# to spec-test.0.wasm, spec-test.1.wasm, etc.
$ out/sexpr-wasm spec-test.wast --spec -o spec-test.json
```

You can use `-h` to get additional help:

```
$ out/sexpr-wasm -h
```

Or try the [online demo](https://cdn.rawgit.com/WebAssembly/sexpr-wasm-prototype/2bb13aa785be9908b95d0e2e09950b39a26004fa/demo/index.html).

## Running wasm-wast

Some examples:

```
# parse binary file test.wasm and write s-expression file test.wast
$ out/wasm-wast test.wasm -o test.wast

# parse test.wasm and write test.wast, using the debug names, if any
$ out/wasm-wast test.wasm --debug-names -o test.wast
```

You can use `-h` to get additional help:

```
$ out/wasm-wast -h
```

## Running wasm-interp

Some examples:

```
# parse binary file test.wasm, and type-check it
$ out/wasm-interp test.wasm

# parse test.wasm and run all its exported functions
$ out/wasm-interp test.wasm --run-all-exports

# parse test.wasm, run the exported functions and trace the output
$ out/wasm-interp test.wasm --run-all-exports --trace

# parse test.json and run the spec tests
$ out/wasm-interp test.json --spec

# parse test.wasm and run all its exported functions, setting the value stack
# size to 100 elements
$ out/wasm-interp test.wasm -V 100 --run-all-exports
```

As a convenience, you can use `test/run-interp.py` to convert a .wast file to
binary first, then run it in the interpreter:

```
$ test/run-interp.py --spec spec-test.wast
20/20 tests.passed.
```

You can use `-h` to get additional help:

```
$ out/wasm-interp -h
$ out/run-interp.py -h
```

## Running WebAssembly in a JavaScript engine

The easiest way is to use the `test/run-js.py` script. This will run
`sexpr-wasm` to convert the .wast to binary, then run all its exported
functions in d8 or spidermonkey:

```
$ test/run-js.py test.wast
test() = 3
```

You can also run spec tests:

```
$ test/run-js.py --spec spec-test.wast
20/20 tests passed.
```

Take a look at `test/spec.js` and `test/wast.js`. These are the JavaScript
scripts that `run-js.py` uses to run WebAssembly.

## Running the test suite

To run all the tests with default configuration:

```
$ make test
```

Every make target has a matching `test-*` target.

```
$ make gcc-debug-asan
$ make test-gcc-debug-asan
$ make clang-release
$ make test-clang-release
...
```

You can also run the Python test runner script directly:

```
$ test/run-tests.py
```

To run a subset of the tests, use a glob-like syntax:

```
$ test/run-tests.py const -v
+ dump/const.txt (0.002s)
+ parse/assert/bad-assertreturn-non-const.txt (0.003s)
+ parse/expr/bad-const-i32-overflow.txt (0.002s)
+ parse/expr/bad-const-f32-trailing.txt (0.004s)
+ parse/expr/bad-const-i32-garbage.txt (0.005s)
+ parse/expr/bad-const-i32-trailing.txt (0.003s)
+ parse/expr/bad-const-i32-underflow.txt (0.003s)
+ parse/expr/bad-const-i64-overflow.txt (0.002s)
+ parse/expr/bad-const-i32-just-negative-sign.txt (0.004s)
+ parse/expr/const.txt (0.002s)
[+10|-0|%100] (0.11s)

$ test/run-tests.py expr*const*i32
+ parse/expr/bad-const-i32-just-negative-sign.txt (0.002s)
+ parse/expr/bad-const-i32-overflow.txt (0.003s)
+ parse/expr/bad-const-i32-underflow.txt (0.002s)
+ parse/expr/bad-const-i32-garbage.txt (0.004s)
+ parse/expr/bad-const-i32-trailing.txt (0.002s)
[+5|-0|%100] (0.11s)
```

When tests are broken, they will give you the expected stdout/stderr as a diff:

```
$ <whoops, turned addition into subtraction in the interpreter>
$ test/run-tests.py interp/binary
- interp/binary.txt
  STDOUT MISMATCH:
  --- expected
  +++ actual
  @@ -1,4 +1,4 @@
  -i32_add() => i32:3
  +i32_add() => i32:4294967295
   i32_sub() => i32:16
   i32_mul() => i32:21
   i32_div_s() => i32:4294967294

**** FAILED ******************************************************************
- interp/binary.txt
[+0|-1|%100] (0.13s)
```

## Writing New Tests

Tests must be placed in the test/ directory, and must have the extension
`.txt`. The directory structure is mostly for convenience, so for example you
can type `test/run-tests.py interp` to run all the interpreter tests.
There's otherwise no logic attached to a test being in a given directory.

That being said, try to make the test names self explanatory, and try to test
only one thing. Also make sure that tests that are expected to fail start with
`bad-`.

The test format is straightforward:

```
;;; KEY1: VALUE1A VALUE1B...
;;; KEY2: VALUE2A VALUE2B...
(input (to)
  (the executable))
(;; STDOUT ;;;
expected stdout
;;; STDOUT ;;)
(;; STDERR ;;;
expected stderr
;;; STDERR ;;)
```

The test runner will copy the input to a temporary file and pass it as an
argument to the executable (which by default is `out/sexpr-wasm`).

The currently supported list of keys:

- `TOOL`: a set of preconfigured keys, see below.
- `EXE`: the executable to run, defaults to out/sexpr-wasm
- `STDIN_FILE`: the file to use for STDIN instead of the contents of this file.
- `FLAGS`: additional flags to pass to the executable
- `ERROR`: the expected return value from the executable, defaults to 0
- `SLOW`: if defined, this test's timeout is doubled.
- `SKIP`: if defined, this test is not run. You can use the value as a comment.
- `TODO`,`NOTE`: useful place to put additional info about the test.

The currently supported list of tools:

- `sexpr-wasm`: runs `sexpr-wasm`
- `run-js`: runs the `run-js.py` script
- `run-js-spec`: runs the `run-js.py` script with `--spec` flag
- `run-roundtrip`: runs the `run-roundtrip.py` script. This does a roundtrip
  conversion using `sexpr-wasm` and `wasm-wast`, making sure the .wasm results
  are identical.
- `run-interp`: runs the `run-interp.py` script, running all exported functions
- `run-interp-spec`: runs the `run-interp.py` script with the `--spec` flag

When you first write a test, it's easiest if you omit the expected stdout and
stderr. You can have the test harness fill it in for you automatically. First
let's write our test:

```
$ cat > test/my-awesome-test.txt << HERE
;;; TOOL: run-js-spec
(module
  (export "add2" 0)
  (func (param i32) (result i32)
    (i32.add (get_local 0) (i32.const 2))))
(assert_return (invoke "add2" (i32.const 4)) (i32.const 6))
(assert_return (invoke "add2" (i32.const -2)) (i32.const 0))
HERE
```

If we run it, it will fail:

```
- my-awesome-test.txt
  STDOUT MISMATCH:
  --- expected
  +++ actual
  @@ -0,0 +1 @@
  +2/2 tests passed.

**** FAILED ******************************************************************
- my-awesome-test.txt
[+0|-1|%100] (0.03s)
```

We can rebase it automatically with the `-r` flag. Running the test again shows
that the expected stdout has been added:

```
$ test/run-tests.py my-awesome-test -r
[+1|-0|%100] (0.03s)
$ test/run-tests.py my-awesome-test
[+1|-0|%100] (0.03s)
$ tail -n 3 test/my-awesome-test.txt
(;; STDOUT ;;;
2/2 tests passed.
;;; STDOUT ;;)
```

## Sanitizers

To build with the [LLVM sanitizers](https://github.com/google/sanitizers),
append the sanitizer name to the target:

```
$ make clang-debug-asan
$ make clang-debug-msan
$ make clang-debug-lsan
$ make clang-debug-ubsan
```

There are configurations for the Address Sanitizer (ASAN), Memory Sanitizer
(MSAN), Leak Sanitizer (LSAN) and Undefine Behavior Sanitizer (UBSAN). You can
read about the behaviors of the sanitizers in the link above, but essentially
the Address Sanitizer finds invalid memory accesses (use after free, access
out-of-bounds, etc.), Memory Sanitizer finds uses of uninitialized memory, 
the Leak Sanitizer finds memory leaks, and the Undefined Behavior Sanitizer
finds undefined behavior (surprise!).

Typically, you'll just want to run all the tests for a given sanitizer:

```
$ make test-asan
```

You can also run the tests for a release build:

```
$ make test-clang-release-asan
...
```

The Travis bots run all of these tests (and more). Before you land a change,
you should run them too. One easy way is to use the `test-everything` target:

```
$ make test-everything
```

To run everything the Travis bots do, you can use the following scripts:

```
$ CC=gcc scripts/travis-build.sh
$ CC=gcc scripts/travis-test.sh
$ CC=clang scripts/travis-build.sh
$ CC=clang scripts/travis-test.sh
```

# Tests

The `test` directory contains all tests, as well as scripts for running the
tests.

All end-to-end tests are written using a text format that is parsed by
`test/run-tests.py`. All files with the extension `.txt` recursively under the
`test` directory will be run as tests.

## Running the test suite

To run all the tests with default configuration:

```console
$ make test
```

Every make target has a matching `test-*` target.

```console
$ make gcc-debug-asan
$ make test-gcc-debug-asan
$ make clang-release
$ make test-clang-release
...
```

You can also run the Python test runner script directly:

```console
$ test/run-tests.py
```

To run a subset of the tests, use a glob-like syntax:

```console
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

$ test/run-tests.py expr*const*i32 -v
+ parse/expr/bad-const-i32-just-negative-sign.txt (0.002s)
+ parse/expr/bad-const-i32-overflow.txt (0.003s)
+ parse/expr/bad-const-i32-underflow.txt (0.002s)
+ parse/expr/bad-const-i32-garbage.txt (0.004s)
+ parse/expr/bad-const-i32-trailing.txt (0.002s)
[+5|-0|%100] (0.11s)
```

When tests are broken, they will give you the expected stdout/stderr as a diff:

```console
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

## Test file format

The test format is straightforward:

```wast
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
argument to the executable (which by default is `out/wat2wasm`).

The currently supported list of keys:

- `TOOL`: a set of preconfigured keys, see below.
- `RUN`: the executable to run, defaults to out/wat2wasm
- `STDIN_FILE`: the file to use for STDIN instead of the contents of this file.
- `ARGS`: additional args to pass to the executable
- `ARGS{N}`: additional args to the Nth `RUN` command (zero-based)
- `ARGS*`: additional args to all `RUN` commands
- `ENV`: environment variables to set, separated by spaces
- `ERROR`: the expected return value from the executable, defaults to 0
- `SLOW`: if defined, this test's timeout is increased (currently by 3x).
- `SKIP`: if defined, this test is not run. You can use the value as a comment.
- `TODO`,`NOTE`: useful place to put additional info about the test.

The currently supported list of tools (see
[run-tests.py](https://github.com/WebAssembly/wabt/blob/master/test/run-tests.py#L44)):

- `wat2wasm`: parse a wasm text file and validate it.
- `wat-desugar`: parse the wasm text file and rewrite it in the canonical text
  format.
- `run-objdump`: parse a wasm text file, convert it to binary, then run
  `wasm-objdump` on it.
- `run-objdump-gen-wasm`: parse a "gen-wasm" text file, convert it to binary,
  then run `wasm-objdump` on it.
- `run-objdump-spec`: parse a wast spec test file, convert it to JSON and a
  collection of `.wasm` files, then run `wasm-objdump`. Note that the `.wasm`
  files are not automatically passed to `wasm-objdump`, so each test must
  specify them manually: `%(temp_file)s.0.wasm %(temp_file)s.1.wasm`, etc.
- `run-roundtrip`: parse a wasm text file, convert it to binary, convert it
  back to text, then finally convert it back to binary and compare the two
  binary results. If the `--stdout` flag is passed, the final conversion to
  binary is skipped and the resulting text is displayed instead.
- `run-interp`: parse a wasm text file, convert it to binary, then run
  `wasm-interp` on this binary, which runs all exported functions in an
  interpreter
- `run-interp-spec`: parse a spec test text file, convert it to a JSON file and
  a collection of `.wasm` and `.wast` files, then run `wasm-interp` on the JSON
  file.
- `run-gen-wasm`: parse a "gen-wasm" text file (which can describe invalid
  binary files), then parse via `wasm2wat` and display the result
- `run-gen-wasm-interp`: parse a "gen-wasm" text file, generate a wasm file,
  the run `wasm-interp` on it, which runes all exported functions in an
  interpreter
- `run-opcodecnt`: parse a wasm text file, convert it to binary, then display
  opcode usage counts.
- `run-gen-spec-js`: parse wasm spec test text file, convert it to a JSON file
  and a collection of `.wasm` and `.wast` files, then take all of these files
  and generate a JavaScript file that will execute the same tests.
- `run-spec-wasm2c`: similar to `run-gen-spec-js`, but the output instead will
  be C source files, that are then compiled with the default C compiler (`cc`).
  Finally, the native executable is run.


## Test subdirectories

Tests must be placed in the test/ directory, and must have the extension
`.txt`. The subdirectory structure is mostly for convenience, so for example
you can type `test/run-tests.py interp` to run all the interpreter tests.
There's otherwise no logic attached to a test being in a given directory.

Here is a brief description of the tests are contained in each top-level
subdirectory:

- `binary`: Tests binary files that are impossible to generate via wat2wasm.
  Typically these are illegal binary files, to ensure binary file reading is
  robust.
- `desugar`: Tests the `wat-desugar` tool.
- `dump`: Tests the verbose output of `wat2wasm` and the output of
  `wasm-objdump`.
- `exceptions`: Tests the new experimental exceptions feature.
- `gen-spec-js`: Tests the gen-spec-js tool, which converts a spec test into a
  JavaScript file.
- `help`: Tests the output of running with the `--help` flag on each tool.
- `interp`: Tests the `wasm-interp` tool.
- `opcodecnt`: Tests the `wasm-opcodecnt` tool.
- `parse`: Tests parsing via the `wat2wasm` tool.
- `regress`: Various regression tests that are irregular and don't fit
  naturally in the other directories.
- `roundtrip`: Tests that roundtripping the text to binary and back to text
  works properly. Also contains tests of the binary reader when the generated
  binary file is valid (if the file is invalid, it will be generated by
  `gen-wasm.py` and should be placed in the `binary` directory instead).
- `spec`: All of the spec core tests. These tests are auto-generated by the
  `update-spec-tests.py` script.
- `typecheck`: Tests the wast validation in the `wat2wasm` tool.

## Writing New Tests

Try to make the test names self explanatory, and try to test only one thing.
Also make sure that tests that are expected to fail start with `bad-`.

When you first write a test, it's easiest if you omit the expected stdout and
stderr. You can have the test harness fill it in for you automatically. First
let's write our test:

```sh
$ cat > test/my-awesome-test.txt << HERE
;;; TOOL: run-interp-spec
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

```console
$ test/run-tests.py my-awesome-test -r
[+1|-0|%100] (0.03s)
$ test/run-tests.py my-awesome-test
[+1|-0|%100] (0.03s)
$ tail -n 3 test/my-awesome-test.txt
(;; STDOUT ;;;
2/2 tests passed.
;;; STDOUT ;;)
```

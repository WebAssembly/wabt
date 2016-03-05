.. image:: https://travis-ci.org/WebAssembly/sexpr-wasm-prototype.svg?branch=master
    :target: https://travis-ci.org/WebAssembly/sexpr-wasm-prototype
    :alt: Build Status

sexpr-wasm
==========

Translates from WebAssembly `s-expressions
<https://github.com/WebAssembly/spec>`_ to the WebAssembly `binary encoding
<https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md>`_.

Cloning
-------

Clone as normal, but don't forget to update/init submodules as well::

  $ git clone https://github.com/WebAssembly/sexpr-wasm-prototype
  $ git submodule update --init

This will fetch the v8 and testsuite repos, which are needed for some tests.

Building
--------

You'll need `CMake<https://cmake.org>`_. If you just run ``make``, it will run
CMake for you, and put the result in ``out/clang/Debug/`` by default::

  $ make
  mkdir -p out/clang/Debug
  cd out/clang/Debug && cmake ../../.. -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
  -- The C compiler identification is Clang 3.4.0
  ...
  make --no-print-directory -C out/clang/Debug sexpr-wasm
  ...
  Linking C executable sexpr-wasm
  [100%] Built target sexpr-wasm
  ln -sf ../out/clang/Debug/sexpr-wasm out/sexpr-wasm

This will build the default version of sexpr-wasm: a debug build using the
Clang compiler. It will also create a symlink to the built binary in
``out/sexpr-wasm``.

There are many make targets available for other configurations as well::

  $ make gcc-release-sexpr-wasm
  ...
  $ make clang-debug-sexpr-wasm-lsan
  ...

You can also run CMake yourself, the normal way::

  $ mkdir build
  $ cd build
  $ cmake ..
  -- The C compiler identification is GNU 4.8.4
  -- The CXX compiler identification is GNU 4.8.4
  -- Check for working C compiler: /usr/bin/cc
  -- Check for working C compiler: /usr/bin/cc -- works
  ...

If you make changes to ``src/wasm-bison-parser.y``, you'll need to install
Bison as well. On Debian-based systems::

  $ sudo apt-get install bison

If you make changes to ``src/wasm-flex-lexer.l``, you'll need to install Flex
as well. On Debian-based systems::

  $ sudo apt-get install flex

CMake will detect if you don't have Flex or Bison installed and use the
prebuilt source files instead.

Building d8
-----------

d8 can load and run the generated binary-encoded files. Some of the tests rely
on this executable. To build it::

  $ scripts/build-d8.sh
  ...

When it is finished, there will be a d8 executable in the
``third_party/v8/v8/out/Release`` directory.

You can also download a prebuilt version (the same one used to test on Travis)
by running the ``download-d8.sh`` script::

  $ scripts/download-d8.sh
  ...

This downloads the d8 executable into the ``out`` directory. The test runner
will look here if there is no d8 executable in the
``third_party/v8/v8/out/Release`` directory.

Running
-------

First write some WebAssembly s-expressions::

  $ cat > test.wast << HERE
  (module
    (export "test" 0)
    (func (result i32)
      (i32.add (i32.const 1) (i32.const 2))))
  HERE

Then run sexpr-wasm to build a binary-encoded file::

  $ out/sexpr-wasm -o test.wasm test.wast

This can be loaded into d8 using JavaScript like this::

  $ third_party/v8/v8/out/Release/d8
  V8 version 4.7.0 (candidate)
  d8> buffer = readbuffer('test.wasm');
  [object ArrayBuffer]
  d8> module = Wasm.instantiateModule(buffer, {});
  {memory: [object ArrayBuffer], test: function test() { [native code] }}
  d8> module.test()
  3

If you just want to run a quick test, you can use the ``run-d8.py`` script
instead::

  $ test/run-d8.py test.wast
  test() = 3

To run spec-style tests (with assert_eq, invoke, etc.) use the ``--spec`` flag::

  $ cat > test2.wast << HERE
  (module
    (export "neg" 0)
    (func (param i32) (result i32)
      (i32.sub (i32.const 0) (get_local 0))))
  (assert_eq (invoke "neg" (i32.const 100)) (i32.const -100))
  HERE
  $ test/run-d8.py --spec test2.wast
  instantiating module
  $assert_eq_0 OK
  1/1 tests passed.

Tests
-----

To run tests::

  $ make test
  [+420|-0|%100] (1.95s)

In this case, there were 420 passed tests and no failed tests, which took 1.95
seconds to run.

You can also run the Python test runner script directly::

  $ test/run-tests.py
  [+420|-0|%100] (1.99s)

  $ test/run-tests.py -v
  . spec/address.txt (skipped)
  . spec/fac.txt (skipped)
  . spec/runaway-recursion.txt (skipped)
  + d8/assertreturn-complex-module.txt (0.044s)
  + d8/assertreturn-invoke-ordering.txt (0.063s)
  + d8/assertreturn-failed.txt (0.068s)
  + d8/assertreturn-types.txt (0.077s)
  + d8/basic.txt (0.060s)
  + d8/assertreturn.txt (0.089s)
  + d8/assertreturnnan.txt (0.069s)
  ...

To run a subset of the tests, use a glob-like syntax::

  $ test/run-tests.py const -v
  + dump/const.txt (0.002s)
  + parse/expr/bad-const-f32-trailing.txt (0.002s)
  + parse/assert/bad-assertreturn-non-const.txt (0.004s)
  + parse/expr/bad-const-i32-garbage.txt (0.003s)
  + parse/expr/bad-const-i32-trailing.txt (0.003s)
  + parse/expr/bad-const-i32-overflow.txt (0.004s)
  + parse/expr/bad-const-i32-underflow.txt (0.002s)
  + parse/expr/bad-const-i32-just-negative-sign.txt (0.006s)
  + parse/expr/bad-const-i64-overflow.txt (0.002s)
  + parse/expr/const.txt (0.002s)
  [+10|-0|%100] (0.01s)

  $ test/run-tests.py expr*const*i32 -v
  + parse/expr/bad-const-i32-garbage.txt (0.003s)
  + parse/expr/bad-const-i32-underflow.txt (0.003s)
  + parse/expr/bad-const-i32-overflow.txt (0.005s)
  + parse/expr/bad-const-i32-just-negative-sign.txt (0.005s)
  + parse/expr/bad-const-i32-trailing.txt (0.005s)
  [+5|-0|%100] (0.01s)

When tests are broken, they will give you the expected stdout/stderr as a diff::

  $ <introduce bug in wasm-binary-writer.c>
  $ test/run-tests.py d8/store
  - d8/store.txt
    STDOUT MISMATCH:
    --- expected
    +++ actual
    @@ -1,9 +1,9 @@
     i32_store8() = -16909061
    -i32_store16() = -859059511
    -i32_store() = -123456
    +i32_store16() = -16909061
    +i32_store() = -16909120
     i64_store8() = -16909061
     i64_store16() = -859059511
    -i64_store32() = -123456
    -i64_store() = 1
    -f32_store() = 1069547520
    -f64_store() = -1064352256
    +i64_store32() = -859059511
    +i64_store() = 0
    +f32_store() = -859059699
    +f64_store() = 61166

  **** FAILED ******************************************************************
  - d8/store.txt
  [+0|-1|%100] (0.03s)

Writing New Tests
-----------------

Tests must be placed in the test/ directory, and must have the extension
``.txt``. The directory structure is mostly for convenience, so for example you
can type ``test/run-tests.py d8`` to run all the tests that execute in d8.
There's otherwise no logic attached to a test being in a given directory.

That being said, try to make the test names self explanatory, and try to test
only one thing. Also make sure that tests that are expected to fail start with
``bad-``.

The test format is straightforward::

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

The test runner will copy the input to a temporary file and pass it as an
argument to the executable (which by default is out/sexpr-wasm).

The currently supported list of keys:

- EXE: the executable to run, defaults to out/sexpr-wasm
- STDIN_FILE: the file to use for STDIN instead of the contents of this file.
- FLAGS: additional flags to pass to the executable
- ERROR: the expected return value from the executable, defaults to 0
- SLOW: if defined, this test's timeout is doubled.
- SKIP: if defined, this test is not run. You can use the value as a comment.
- TODO,NOTE: useful place to put additional info about the test.

When you first write a test, it's easiest if you omit the expected stdout and
stderr. You can have the test harness fill it in for you automatically. First
let's write our test::

  $ cat > test/my-awesome-test.txt << HERE
  ;;; EXE: test/run-d8.py
  ;;; FLAGS: --spec
  (module
    (export "add2" 0)
    (func (param i32) (result i32)
      (i32.add (get_local 0) (i32.const 2))))
  (assert_return (invoke "add2" (i32.const 4)) (i32.const 6))
  (assert_return (invoke "add2" (i32.const -2)) (i32.const 0))
  HERE

If we run it, it will fail::

  - my-awesome-test.txt
    STDOUT MISMATCH:
    --- expected
    +++ actual
    @@ -0,0 +1 @@
    +2/2 tests passed.

  **** FAILED ******************************************************************
  - my-awesome-test.txt
  [+0|-1|%100] (0.03s)

We can rebase it automatically with the ``-r`` flag. Running the test again
shows that the expected stdout has been added::

  $ test/run-tests.py my-awesome-test -r
  [+1|-0|%100] (0.03s)
  $ test/run-tests.py my-awesome-test
  [+1|-0|%100] (0.03s)
  $ tail -n 3 test/my-awesome-test.txt
  (;; STDOUT ;;;
  2/2 tests passed.
  ;;; STDOUT ;;)

Sanitizers
----------

To build with the `LLVM sanitizers <https://github.com/google/sanitizers>`_,
append the sanitizer name to sexpr-wasm::

  $ make clang-debug-sexpr-wasm-asan
  ...
  $ make clang-debug-sexpr-wasm-msan
  ...
  $ make clang-debug-sexpr-wasm-lsan
  ...

There are configurations for the Address Sanitizer (ASAN), Memory Sanitizer
(MSAN) and Leak Sanitizer (LSAN). You can read about the behaviors of the
sanitizers in the link above, but essentially the Address Sanitizer finds
invalid memory accesses (use after free, access out-of-bounds, etc.), Memory
Sanitizer finds uses of uninitialized memory, and the Leak Sanitizer finds
memory leaks.

Typically, you'll just want to run all the tests for a given sanitizer::

  $ make test-asan
  [+420|-0|%100] (12.59s)

You can also run the tests for a release build::

  $ make test-clang-release-asan
  ...

The Travis bots run all of these tests. Before you land a change, you should
run them too. One easy way is to use the ``test-everything`` target::

  $ make test-everything
  [+420|-0|%100] (1.71s)
  [+420|-0|%100] (12.20s)
  [+420|-0|%100] (4.71s)
  [+420|-0|%100] (5.52s)

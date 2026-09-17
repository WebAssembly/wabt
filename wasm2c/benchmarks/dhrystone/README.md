# Dhrystone benchmark

This standalone benchmark translates a WASIp1 module with `wasm2c` and supplies
its own host functions using uvwasi. It does not use `wasm-interp`.

Initialize the repository submodules, then build the tools and the benchmark's
uvwasi/libuv dependencies from the WABT repository root:

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_UVWASI=ON
cmake --build build --target wasm2c wasm2c-copy-to-bin uvwasi_a
```

The explicit `uvwasi_a` target also builds `uv_a`. These optional libraries are
not built by the default target and are not linked into `wasm-interp`.
`BUILD_UVWASI` replaces the benchmark's former reliance on `WITH_WASI`.

The Makefile expects Clang on `PATH`, a WASI SDK at `/opt/wasi-sdk`, and the
libraries in the root `build` directory. With those prerequisites available:

```sh
cd wasm2c/benchmarks/dhrystone
make
```

The `dhrystone_segue` target additionally requires an x86-64 host with FSGSBASE
support.

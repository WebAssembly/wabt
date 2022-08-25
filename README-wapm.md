# WABT: The WebAssembly Binary Toolkit

You can install The WebAssembly Binary Toolkit with:

```shell
wapm install wabt
```

wabt includes commands like:

 - **wat2wasm**
 - **wasm2wat**
 - **wast2json**
 - **wasm-validate**
 - **wasm-strip**


## Commands

### wat2wasm

Translate from [WebAssembly text format](https://webassembly.github.io/spec/core/text/index.html) to the [WebAssembly binary format](https://webassembly.github.io/spec/core/binary/index.html).

```shell
# parse and typecheck test.wat
$ wapm run wat2wasm test.wat

# parse test.wat and write to binary file test.wasm
$ wapm run wat2wasm test.wat -o test.wasm

# parse spec-test.wast, and write verbose output to stdout (including the
# meaning of every byte)
$ wapm run wat2wasm spec-test.wast -v
```

### wasm2wat

The inverse of wat2wasm, translate from the binary format back to the text format (also known as a .wat).

```shell
# parse and typecheck test.wat
$ wapm run wat2wasm test.wat

# parse test.wat and write to binary file test.wasm
$ wapm run wat2wasm test.wat -o test.wasm

# parse spec-test.wast, and write verbose output to stdout (including the
# meaning of every byte)
$ wapm run wat2wasm spec-test.wast -v
```

### wasm-validate

Validate a file in the WebAssembly binary format

```shell
# Validates the WebAssembly file
$ wapm run wasm-validate test.wasm
```

### wasm-strip

Remove sections of a WebAssembly binary file

```shell
# Validates the WebAssembly file
$ wapm run wasm-strip test.wasm
```

### wast2json

wast2json converts a file in the wasm spec test format to a JSON file and associated wasm binary files

```shell
# parse spec-test.wast, and write files to spec-test.json. Modules are written
# to spec-test.0.wasm, spec-test.1.wasm, etc.
$ wapm run wast2json spec-test.wast -o spec-test.json
```


## Building from Source

1. Run `./scripts/build_wasi.sh`

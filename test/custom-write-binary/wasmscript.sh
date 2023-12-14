#!/bin/bash

# Get the path where the current script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Splicing path
WASM_FILE="${SCRIPT_DIR}/hello.wasm"
WASM_CONTRAST="${SCRIPT_DIR}/output.wasm"
WAT_FILE="${SCRIPT_DIR}/hello.wat"
BINARY_FILE="${SCRIPT_DIR}/global_vars.bin"

# Execute the wasm2wat command
wasm2wat "${WASM_FILE}" --enable-annotations -o "${WAT_FILE}"

# Execute the wat2wasm command
wat2wasm "${WAT_FILE}" --enable-annotations -c my_custom_section -p "${BINARY_FILE}" -o "${WASM_CONTRAST}"

# View the structure of the newly generated wasm file
wasm-objdump -h output.wasm

# New execution of wasm2wat command
wasm2wat "${WASM_CONTRAST}" --enable-annotations -o "output.wat" 

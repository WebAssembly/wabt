#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd $SCRIPT_DIR

wit-bindgen c --export bindings.wit --out-dir $SCRIPT_DIR

# Note: WABT has set up the include path so all #includes need to be relative to
# the project root
sed -i 's|#include <bindings.h>|#include "src/embedding/bindings.h"|g' bindings.c

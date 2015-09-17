#ifndef WASM_GEN_H
#define WASM_GEN_H

struct WasmTokenizer;

void wasm_gen_file(struct WasmTokenizer* tokenizer, int multi_module);

#endif /* WASM_GEN_H */

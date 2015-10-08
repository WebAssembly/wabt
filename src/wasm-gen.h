#ifndef WASM_GEN_H
#define WASM_GEN_H

int wasm_gen_file(struct WasmSource* source,
                  int multi_module,
                  enum WasmParserTypeCheck type_check);

#endif /* WASM_GEN_H */

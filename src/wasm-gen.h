#ifndef WASM_GEN_H
#define WASM_GEN_H

typedef struct WasmGenOptions {
  const char* outfile;
  int dump_module;
  int verbose;
  int multi_module;
  int multi_module_verbose;
  enum WasmParserTypeCheck type_check;
} WasmGenOptions;

int wasm_gen_file(struct WasmSource* source, WasmGenOptions* options);

#endif /* WASM_GEN_H */

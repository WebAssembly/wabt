#ifndef WASM_GEN_H
#define WASM_GEN_H

typedef struct WasmGenOptions {
  const char* outfile;
  int dump_module;
  int verbose;
  int spec;
  int spec_verbose;
} WasmGenOptions;

int wasm_gen_file(struct WasmSource* source, WasmGenOptions* options);

#endif /* WASM_GEN_H */

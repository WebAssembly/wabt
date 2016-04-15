/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WASM_BINARY_READER_H_
#define WASM_BINARY_READER_H_

#include <stddef.h>
#include <stdint.h>

#include "wasm-binary.h"
#include "wasm-common.h"

struct WasmAllocator;

#define WASM_READ_BINARY_OPTIONS_DEFAULT \
  { WASM_FALSE }

typedef struct WasmReadBinaryOptions {
  WasmBool read_debug_names;
} WasmReadBinaryOptions;

typedef struct WasmBinaryReader {
  void* user_data;

  void (*on_error)(uint32_t offset, const char* message, void* user_data);

  /* module */
  WasmResult (*begin_module)(void* user_data);
  WasmResult (*end_module)(void* user_data);

  /* memory section */
  WasmResult (*begin_memory_section)(void* user_data);
  WasmResult (*on_memory_initial_size_pages)(uint32_t pages, void* user_data);
  WasmResult (*on_memory_max_size_pages)(uint32_t pages, void* user_data);
  WasmResult (*on_memory_exported)(WasmBool exported, void* user_data);
  WasmResult (*end_memory_section)(void* user_data);

  /* data segment section */
  WasmResult (*begin_data_segment_section)(void* user_data);
  WasmResult (*on_data_segment_count)(uint32_t count, void* user_data);
  WasmResult (*on_data_segment)(uint32_t index,
                                uint32_t address,
                                const void* data,
                                uint32_t size,
                                void* user_data);
  WasmResult (*end_data_segment_section)(void* user_data);

  /* signatures section */
  WasmResult (*begin_signature_section)(void* user_data);
  WasmResult (*on_signature_count)(uint32_t count, void* user_data);
  WasmResult (*on_signature)(uint32_t index,
                             uint32_t param_count,
                             WasmType* param_types,
                             uint32_t result_count,
                             WasmType* result_types,
                             void* user_data);
  WasmResult (*end_signature_section)(void* user_data);

  /* imports section */
  WasmResult (*begin_import_section)(void* user_data);
  WasmResult (*on_import_count)(uint32_t count, void* user_data);
  WasmResult (*on_import)(uint32_t index,
                          uint32_t sig_index,
                          WasmStringSlice module_name,
                          WasmStringSlice function_name,
                          void* user_data);
  WasmResult (*end_import_section)(void* user_data);

  /* function signatures section */
  WasmResult (*begin_function_signatures_section)(void* user_data);
  WasmResult (*on_function_signatures_count)(uint32_t count, void* user_data);
  WasmResult (*on_function_signature)(uint32_t index,
                                      uint32_t sig_index,
                                      void* user_data);
  WasmResult (*end_function_signatures_section)(void* user_data);

  /* function bodies section */
  WasmResult (*begin_function_bodies_section)(void* user_data);
  WasmResult (*on_function_bodies_count)(uint32_t count, void* user_data);
  WasmResult (*begin_function_body)(uint32_t index, void* user_data);
  WasmResult (*on_local_decl_count)(uint32_t count, void* user_data);
  WasmResult (*on_local_decl)(uint32_t decl_index,
                              uint32_t count,
                              WasmType type,
                              void* user_data);

  /* function expressions; called between begin_function_body and
   end_function_body */
  WasmResult (*on_binary_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_block_expr)(int rtn_first, uint32_t count,
                              void* user_data);
  WasmResult (*on_br_expr)(uint32_t depth, void* user_data);
  WasmResult (*on_br_if_expr)(uint32_t depth, void* user_data);
  WasmResult (*on_br_table_expr)(uint32_t num_targets,
                                 uint32_t* target_depths,
                                 uint32_t default_target_depth,
                                 void* user_data);
  WasmResult (*on_call_expr)(uint32_t func_index, void* user_data);
  WasmResult (*on_call_import_expr)(uint32_t import_index, void* user_data);
  WasmResult (*on_call_indirect_expr)(uint32_t sig_index, void* user_data);
  WasmResult (*on_values_expr)(uint32_t values_count, void* user_data);
  WasmResult (*on_conc_values_expr)(uint32_t values_count, void* user_data);
  WasmResult (*on_mv_call_expr)(uint32_t func_index, void* user_data);
  WasmResult (*on_compare_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_i32_const_expr)(uint32_t value, void* user_data);
  WasmResult (*on_i64_const_expr)(uint64_t value, void* user_data);
  WasmResult (*on_f32_const_expr)(uint32_t value_bits, void* user_data);
  WasmResult (*on_f64_const_expr)(uint64_t value_bits, void* user_data);
  WasmResult (*on_convert_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_get_local_expr)(uint32_t local_index, void* user_data);
  WasmResult (*on_grow_memory_expr)(void* user_data);
  WasmResult (*on_if_expr)(void* user_data);
  WasmResult (*on_if_else_expr)(void* user_data);
  WasmResult (*on_load_expr)(WasmOpcode opcode,
                             uint32_t alignment_log2,
                             uint32_t offset,
                             void* user_data);
  WasmResult (*on_loop_expr)(uint32_t count, void* user_data);
  WasmResult (*on_memory_size_expr)(void* user_data);
  WasmResult (*on_nop_expr)(void* user_data);
  WasmResult (*on_return_expr)(void* user_data);
  WasmResult (*on_select_expr)(void* user_data);
  WasmResult (*on_set_local_expr)(uint32_t local_index, void* user_data);
  WasmResult (*on_store_expr)(WasmOpcode opcode,
                              uint32_t alignment_log2,
                              uint32_t offset,
                              void* user_data);
  WasmResult (*on_unary_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_unreachable_expr)(void* user_data);
  WasmResult (*end_function_body)(uint32_t index, void* user_data);
  WasmResult (*end_function_bodies_section)(void* user_data);

  /* function table section */
  WasmResult (*begin_function_table_section)(void* user_data);
  WasmResult (*on_function_table_count)(uint32_t count, void* user_data);
  WasmResult (*on_function_table_entry)(uint32_t index,
                                        uint32_t func_index,
                                        void* user_data);
  WasmResult (*end_function_table_section)(void* user_data);

  /* start section */
  WasmResult (*begin_start_section)(void* user_data);
  WasmResult (*on_start_function)(uint32_t func_index, void* user_data);
  WasmResult (*end_start_section)(void* user_data);

  /* exports section */
  WasmResult (*begin_export_section)(void* user_data);
  WasmResult (*on_export_count)(uint32_t count, void* user_data);
  WasmResult (*on_export)(uint32_t index,
                          uint32_t func_index,
                          WasmStringSlice name,
                          void* user_data);
  WasmResult (*end_export_section)(void* user_data);

  /* names section */
  WasmResult (*begin_names_section)(void* user_data);
  WasmResult (*on_function_names_count)(uint32_t count, void* user_data);
  WasmResult (*on_function_name)(uint32_t index,
                                 WasmStringSlice name,
                                 void* user_data);
  WasmResult (*on_local_names_count)(uint32_t index,
                                     uint32_t count,
                                     void* user_data);
  WasmResult (*on_local_name)(uint32_t func_index,
                              uint32_t local_index,
                              WasmStringSlice name,
                              void* user_data);
  WasmResult (*end_names_section)(void* user_data);

} WasmBinaryReader;

WASM_EXTERN_C_BEGIN
WasmResult wasm_read_binary(struct WasmAllocator* allocator,
                            const void* data,
                            size_t size,
                            WasmBinaryReader* reader,
                            const WasmReadBinaryOptions* options);
WASM_EXTERN_C_END

#endif /* WASM_BINARY_READER_H_ */

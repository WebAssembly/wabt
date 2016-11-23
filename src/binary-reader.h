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

#include "binary.h"
#include "common.h"

struct WasmAllocator;

#define WASM_READ_BINARY_OPTIONS_DEFAULT \
  { NULL, WASM_FALSE }

typedef struct WasmReadBinaryOptions {
  struct WasmStream* log_stream;
  WasmBool read_debug_names;
} WasmReadBinaryOptions;

typedef struct WasmBinaryReaderContext {
  const uint8_t* data;
  size_t size;
  size_t offset;
  void* user_data;
} WasmBinaryReaderContext;

typedef struct WasmBinaryReader {
  void* user_data;

  void (*on_error)(WasmBinaryReaderContext* ctx, const char* message);

  /* module */
  WasmResult (*begin_module)(uint32_t version, void* user_data);
  WasmResult (*end_module)(void* user_data);

  /* user section */
  WasmResult (*begin_user_section)(WasmBinaryReaderContext* ctx,
                                   uint32_t size,
                                   WasmStringSlice section_name);
  WasmResult (*end_user_section)(WasmBinaryReaderContext* ctx);

  /* signatures section */
  /* TODO(binji): rename to "type" section */
  WasmResult (*begin_signature_section)(WasmBinaryReaderContext* ctx,
                                        uint32_t size);
  WasmResult (*on_signature_count)(uint32_t count, void* user_data);
  WasmResult (*on_signature)(uint32_t index,
                             uint32_t param_count,
                             WasmType* param_types,
                             uint32_t result_count,
                             WasmType* result_types,
                             void* user_data);
  WasmResult (*end_signature_section)(WasmBinaryReaderContext* ctx);

  /* import section */
  WasmResult (*begin_import_section)(WasmBinaryReaderContext* ctx,
                                     uint32_t size);
  WasmResult (*on_import_count)(uint32_t count, void* user_data);
  WasmResult (*on_import)(uint32_t index,
                          WasmStringSlice module_name,
                          WasmStringSlice field_name,
                          void* user_data);
  WasmResult (*on_import_func)(uint32_t index,
                               uint32_t sig_index,
                               void* user_data);
  WasmResult (*on_import_table)(uint32_t index,
                                WasmType elem_type,
                                const WasmLimits* elem_limits,
                                void* user_data);
  WasmResult (*on_import_memory)(uint32_t index,
                                 const WasmLimits* page_limits,
                                 void* user_data);
  WasmResult (*on_import_global)(uint32_t index,
                                 WasmType type,
                                 WasmBool mutable_,
                                 void* user_data);
  WasmResult (*end_import_section)(WasmBinaryReaderContext* ctx);

  /* function signatures section */
  /* TODO(binji): rename to "function" section */
  WasmResult (*begin_function_signatures_section)(WasmBinaryReaderContext* ctx,
                                                  uint32_t size);
  WasmResult (*on_function_signatures_count)(uint32_t count, void* user_data);
  WasmResult (*on_function_signature)(uint32_t index,
                                      uint32_t sig_index,
                                      void* user_data);
  WasmResult (*end_function_signatures_section)(WasmBinaryReaderContext* ctx);

  /* table section */
  WasmResult (*begin_table_section)(WasmBinaryReaderContext* ctx,
                                    uint32_t size);
  WasmResult (*on_table_count)(uint32_t count, void* user_data);
  WasmResult (*on_table)(uint32_t index,
                         WasmType elem_type,
                         const WasmLimits* elem_limits,
                         void* user_data);
  WasmResult (*end_table_section)(WasmBinaryReaderContext* ctx);

  /* memory section */
  WasmResult (*begin_memory_section)(WasmBinaryReaderContext* ctx,
                                     uint32_t size);
  WasmResult (*on_memory_count)(uint32_t count, void* user_data);
  WasmResult (*on_memory)(uint32_t index,
                          const WasmLimits* limits,
                          void* user_data);
  WasmResult (*end_memory_section)(WasmBinaryReaderContext* ctx);

  /* global section */
  WasmResult (*begin_global_section)(WasmBinaryReaderContext* ctx,
                                     uint32_t size);
  WasmResult (*on_global_count)(uint32_t count, void* user_data);
  WasmResult (*begin_global)(uint32_t index,
                             WasmType type,
                             WasmBool mutable_,
                             void* user_data);
  WasmResult (*begin_global_init_expr)(uint32_t index, void* user_data);
  WasmResult (*end_global_init_expr)(uint32_t index, void* user_data);
  WasmResult (*end_global)(uint32_t index, void* user_data);
  WasmResult (*end_global_section)(WasmBinaryReaderContext* ctx);

  /* exports section */
  WasmResult (*begin_export_section)(WasmBinaryReaderContext* ctx,
                                     uint32_t size);
  WasmResult (*on_export_count)(uint32_t count, void* user_data);
  WasmResult (*on_export)(uint32_t index,
                          WasmExternalKind kind,
                          uint32_t item_index,
                          WasmStringSlice name,
                          void* user_data);
  WasmResult (*end_export_section)(WasmBinaryReaderContext* ctx);

  /* start section */
  WasmResult (*begin_start_section)(WasmBinaryReaderContext* ctx,
                                    uint32_t size);
  WasmResult (*on_start_function)(uint32_t func_index, void* user_data);
  WasmResult (*end_start_section)(WasmBinaryReaderContext* ctx);

  /* function bodies section */
  /* TODO(binji): rename to code section */
  WasmResult (*begin_function_bodies_section)(WasmBinaryReaderContext* ctx,
                                              uint32_t size);
  WasmResult (*on_function_bodies_count)(uint32_t count, void* user_data);
  WasmResult (*begin_function_body_pass)(uint32_t index,
                                         uint32_t pass,
                                         void* user_data);
  WasmResult (*begin_function_body)(uint32_t index, void* user_data);
  WasmResult (*on_local_decl_count)(uint32_t count, void* user_data);
  WasmResult (*on_local_decl)(uint32_t decl_index,
                              uint32_t count,
                              WasmType type,
                              void* user_data);

  /* function expressions; called between begin_function_body and
   end_function_body */
  WasmResult (*on_opcode)(WasmBinaryReaderContext* ctx, WasmOpcode Opcode);
  WasmResult (*on_opcode_bare)(WasmBinaryReaderContext* ctx);
  WasmResult (*on_opcode_uint32)(WasmBinaryReaderContext* ctx, uint32_t value);
  WasmResult (*on_opcode_uint32_uint32)(WasmBinaryReaderContext* ctx,
                                        uint32_t value,
                                        uint32_t value2);
  WasmResult (*on_opcode_uint64)(WasmBinaryReaderContext* ctx, uint64_t value);
  WasmResult (*on_opcode_f32)(WasmBinaryReaderContext* ctx, float value);
  WasmResult (*on_opcode_f64)(WasmBinaryReaderContext* ctx, double value);
  WasmResult (*on_opcode_block_sig)(WasmBinaryReaderContext* ctx,
                                    uint32_t num_types,
                                    WasmType* sig_types);
  WasmResult (*on_binary_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_block_expr)(uint32_t num_types,
                              WasmType* sig_types,
                              void* user_data);
  WasmResult (*on_br_expr)(uint32_t depth, void* user_data);
  WasmResult (*on_br_if_expr)(uint32_t depth, void* user_data);
  WasmResult (*on_br_table_expr)(WasmBinaryReaderContext* ctx,
                                 uint32_t num_targets,
                                 uint32_t* target_depths,
                                 uint32_t default_target_depth);
  WasmResult (*on_call_expr)(uint32_t func_index, void* user_data);
  WasmResult (*on_call_import_expr)(uint32_t import_index, void* user_data);
  WasmResult (*on_call_indirect_expr)(uint32_t sig_index, void* user_data);
  WasmResult (*on_compare_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_convert_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_drop_expr)(void* user_data);
  WasmResult (*on_else_expr)(void* user_data);
  WasmResult (*on_end_expr)(void* user_data);
  WasmResult (*on_f32_const_expr)(uint32_t value_bits, void* user_data);
  WasmResult (*on_f64_const_expr)(uint64_t value_bits, void* user_data);
  WasmResult (*on_get_global_expr)(uint32_t global_index, void* user_data);
  WasmResult (*on_get_local_expr)(uint32_t local_index, void* user_data);
  WasmResult (*on_grow_memory_expr)(void* user_data);
  WasmResult (*on_i32_const_expr)(uint32_t value, void* user_data);
  WasmResult (*on_i64_const_expr)(uint64_t value, void* user_data);
  WasmResult (*on_if_expr)(uint32_t num_types,
                           WasmType* sig_types,
                           void* user_data);
  WasmResult (*on_load_expr)(WasmOpcode opcode,
                             uint32_t alignment_log2,
                             uint32_t offset,
                             void* user_data);
  WasmResult (*on_loop_expr)(uint32_t num_types,
                             WasmType* sig_types,
                             void* user_data);
  WasmResult (*on_current_memory_expr)(void* user_data);
  WasmResult (*on_nop_expr)(void* user_data);
  WasmResult (*on_return_expr)(void* user_data);
  WasmResult (*on_select_expr)(void* user_data);
  WasmResult (*on_set_global_expr)(uint32_t global_index, void* user_data);
  WasmResult (*on_set_local_expr)(uint32_t local_index, void* user_data);
  WasmResult (*on_store_expr)(WasmOpcode opcode,
                              uint32_t alignment_log2,
                              uint32_t offset,
                              void* user_data);
  WasmResult (*on_tee_local_expr)(uint32_t local_index, void* user_data);
  WasmResult (*on_unary_expr)(WasmOpcode opcode, void* user_data);
  WasmResult (*on_unreachable_expr)(void* user_data);
  WasmResult (*end_function_body)(uint32_t index, void* user_data);
  WasmResult (*end_function_body_pass)(uint32_t index,
                                       uint32_t pass,
                                       void* user_data);
  WasmResult (*end_function_bodies_section)(WasmBinaryReaderContext* ctx);

  /* elem section */
  WasmResult (*begin_elem_section)(WasmBinaryReaderContext* ctx, uint32_t size);
  WasmResult (*on_elem_segment_count)(uint32_t count, void* user_data);
  WasmResult (*begin_elem_segment)(uint32_t index,
                                   uint32_t table_index,
                                   void* user_data);
  WasmResult (*begin_elem_segment_init_expr)(uint32_t index, void* user_data);
  WasmResult (*end_elem_segment_init_expr)(uint32_t index, void* user_data);
  WasmResult (*on_elem_segment_function_index_count)(uint32_t index,
                                                     uint32_t count,
                                                     void* user_data);
  WasmResult (*on_elem_segment_function_index)(uint32_t index,
                                               uint32_t func_index,
                                               void* user_data);
  WasmResult (*end_elem_segment)(uint32_t index, void* user_data);
  WasmResult (*end_elem_section)(WasmBinaryReaderContext* ctx);

  /* data section */
  WasmResult (*begin_data_section)(WasmBinaryReaderContext* ctx, uint32_t size);
  WasmResult (*on_data_segment_count)(uint32_t count, void* user_data);
  WasmResult (*begin_data_segment)(uint32_t index,
                                   uint32_t memory_index,
                                   void* user_data);
  WasmResult (*begin_data_segment_init_expr)(uint32_t index, void* user_data);
  WasmResult (*end_data_segment_init_expr)(uint32_t index, void* user_data);
  WasmResult (*on_data_segment_data)(uint32_t index,
                                     const void* data,
                                     uint32_t size,
                                     void* user_data);
  WasmResult (*end_data_segment)(uint32_t index, void* user_data);
  WasmResult (*end_data_section)(WasmBinaryReaderContext* ctx);

  /* names section */
  WasmResult (*begin_names_section)(WasmBinaryReaderContext* ctx,
                                    uint32_t size);
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
  WasmResult (*end_names_section)(WasmBinaryReaderContext* ctx);

  /* init_expr - used by elem, data and global sections; these functions are
   * only called between calls to begin_*_init_expr and end_*_init_expr */
  WasmResult (*on_init_expr_f32_const_expr)(uint32_t index,
                                            uint32_t value,
                                            void* user_data);
  WasmResult (*on_init_expr_f64_const_expr)(uint32_t index,
                                            uint64_t value,
                                            void* user_data);
  WasmResult (*on_init_expr_get_global_expr)(uint32_t index,
                                             uint32_t global_index,
                                             void* user_data);
  WasmResult (*on_init_expr_i32_const_expr)(uint32_t index,
                                            uint32_t value,
                                            void* user_data);
  WasmResult (*on_init_expr_i64_const_expr)(uint32_t index,
                                            uint64_t value,
                                            void* user_data);
} WasmBinaryReader;

WASM_EXTERN_C_BEGIN
WasmResult wasm_read_binary(struct WasmAllocator* allocator,
                            const void* data,
                            size_t size,
                            WasmBinaryReader* reader,
                            uint32_t num_function_passes,
                            const WasmReadBinaryOptions* options);
WASM_EXTERN_C_END

#endif /* WASM_BINARY_READER_H_ */

#ifndef WASM_GEN_H
#define WASM_GEN_H

/* TODO(binji): this shouldn't need to be exposed. Move it back in wasm-gen.c
 later */
typedef struct OutputBuffer {
  void* start;
  size_t size;
  size_t capacity;
} OutputBuffer;
/* TODO(binji): another hack */
OutputBuffer* hack_get_output_buffer(struct Parser* parser);

void out_u8(OutputBuffer*, uint8_t value, const char* desc);
void out_u16(OutputBuffer*, uint16_t value, const char* desc);
void out_u32(OutputBuffer*, uint32_t value, const char* desc);
void out_u64(OutputBuffer*,  uint64_t value, const char* desc);
void out_f32(OutputBuffer*, float value, const char* desc);
void out_f64(OutputBuffer*,  double value, const char* desc);
void out_u8_at(OutputBuffer*, uint32_t offset, uint8_t value, const char* desc);
void out_u32_at(OutputBuffer*,
                uint32_t offset,
                uint32_t value,
                const char* desc);
void out_opcode(OutputBuffer* buf, Opcode opcode);
void out_leb128(OutputBuffer* buf, uint32_t value, const char* desc);

void gen_file(Tokenizer* tokenizer);

#endif /* WASM_GEN_H */

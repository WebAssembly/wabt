#ifndef WASM_VECTOR_H_
#define WASM_VECTOR_H_

#include <stdlib.h>

#define DECLARE_VECTOR(name, type)                               \
  typedef struct type##Vector {                                  \
    type* data;                                                  \
    size_t size;                                                 \
    size_t capacity;                                             \
  } type##Vector;                                                \
  EXTERN_C void wasm_destroy_##name##_vector(type##Vector* vec); \
  EXTERN_C type* wasm_append_##name(type##Vector* vec);          \
  EXTERN_C int wasm_extend_##name##s(type##Vector* dst, type##Vector* src);

#define DEFINE_VECTOR(name, type)                                              \
  void wasm_destroy_##name##_vector(type##Vector* vec) { free(vec->data); }    \
  type* wasm_append_##name(type##Vector* vec) {                                \
    return append_element((void**)&vec->data, &vec->size, &vec->capacity,      \
                          sizeof(type));                                       \
  }                                                                            \
  int wasm_extend_##name##s(type##Vector* dst, type##Vector* src) {            \
    return extend_elements((void**)&dst->data, &dst->size, &dst->capacity,     \
                           (const void**)&src->data, src->size, sizeof(type)); \
  }

void* append_element(void** data,
                     size_t* size,
                     size_t* capacity,
                     size_t elt_byte_size);

int extend_elements(void** dst,
                    size_t* dst_size,
                    size_t* dst_capacity,
                    const void** src,
                    size_t src_size,
                    size_t elt_byte_size);

#endif /* WASM_VECTOR_H_ */

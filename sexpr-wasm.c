#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABS_TO_SPACES 8
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define STATIC_ASSERT__(x, c) typedef char static_assert_##c[x ? 1 : -1]
#define STATIC_ASSERT_(x, c) STATIC_ASSERT__(x, c)
#define STATIC_ASSERT(x) STATIC_ASSERT_(x, __COUNTER__)

#define INITIAL_OUTPUT_BUFFER_CAPACITY (64 * 1024)
/* TODO(binji): read these from the file? use flags? */
#define DEFAULT_MEMORY_SIZE_LOG2 20
#define DEFAULT_MEMORY_EXPORT 1

#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

/* These values match the v8-native-prototype's type's values */
typedef enum Type {
  TYPE_VOID,
  TYPE_I32,
  TYPE_I64,
  TYPE_F32,
  TYPE_F64,
  NUM_TYPES,
} Type;

typedef enum MemType {
  MEM_TYPE_I8,
  MEM_TYPE_I16,
  MEM_TYPE_I32,
  MEM_TYPE_I64,
  MEM_TYPE_F32,
  MEM_TYPE_F64,
} MemType;

typedef enum OpType {
  OP_NONE,
  OP_ASSERT_EQ,
  OP_ASSERT_INVALID,
  OP_BINARY,
  OP_BLOCK,
  OP_BREAK,
  OP_CALL,
  OP_CALL_INDIRECT,
  OP_COMPARE,
  OP_CONST,
  OP_CONVERT,
  OP_DESTRUCT,
  OP_EXPORT,
  OP_FUNC,
  OP_GET_LOCAL,
  OP_GLOBAL,
  OP_IF,
  OP_INVOKE,
  OP_LABEL,
  OP_LOAD,
  OP_LOAD_GLOBAL,
  OP_LOCAL,
  OP_LOOP,
  OP_MEMORY,
  OP_MODULE,
  OP_NOP,
  OP_PARAM,
  OP_RESULT,
  OP_RETURN,
  OP_SET_LOCAL,
  OP_STORE,
  OP_STORE_GLOBAL,
  OP_SWITCH,
  OP_TABLE,
  OP_UNARY,
} OpType;

#define OPCODES(V)             \
  V(NOP, 0x00)                 \
  V(IF, 0x01)                  \
  V(IF_THEN, 0x02)             \
  V(BLOCK, 0x03)               \
  V(SWITCH, 0x04)              \
  V(SWITCH_NF, 0x05)           \
  V(LOOP, 0x06)                \
  V(CONTINUE, 0x07)            \
  V(BREAK, 0x08)               \
  V(RETURN, 0x09)              \
  V(I8_CONST, 0x10)            \
  V(I32_CONST, 0x11)           \
  V(I64_CONST, 0x12)           \
  V(F64_CONST, 0x13)           \
  V(F32_CONST, 0x14)           \
  V(GET_LOCAL, 0x15)           \
  V(SET_LOCAL, 0x16)           \
  V(GET_GLOBAL, 0x17)          \
  V(SET_GLOBAL, 0x18)          \
  V(CALL, 0x19)                \
  V(CALL_INDIRECT, 0x1a)       \
  V(TERNARY, 0x1b)             \
  V(COMMA, 0x1c)               \
  V(I32_LOAD_I32, 0x20)        \
  V(I64_LOAD_I32, 0x21)        \
  V(F32_LOAD_I32, 0x22)        \
  V(F64_LOAD_I32, 0x23)        \
  V(I32_LOAD_I64, 0x24)        \
  V(I64_LOAD_I64, 0x25)        \
  V(F32_LOAD_I64, 0x26)        \
  V(F64_LOAD_I64, 0x27)        \
  V(I32_STORE_I32, 0x30)       \
  V(I64_STORE_I32, 0x31)       \
  V(F32_STORE_I32, 0x32)       \
  V(F64_STORE_I32, 0x33)       \
  V(I32_STORE_I64, 0x34)       \
  V(I64_STORE_I64, 0x35)       \
  V(F32_STORE_I64, 0x36)       \
  V(F64_STORE_I64, 0x37)       \
  V(I32_ADD, 0x40)             \
  V(I32_SUB, 0x41)             \
  V(I32_MUL, 0x42)             \
  V(I32_SDIV, 0x43)            \
  V(I32_UDIV, 0x44)            \
  V(I32_SREM, 0x45)            \
  V(I32_UREM, 0x46)            \
  V(I32_AND, 0x47)             \
  V(I32_OR, 0x48)              \
  V(I32_XOR, 0x49)             \
  V(I32_SHL, 0x4a)             \
  V(I32_SHR, 0x4b)             \
  V(I32_SAR, 0x4c)             \
  V(I32_EQ, 0x4d)              \
  V(I32_NE, 0x4e)              \
  V(I32_SLT, 0x4f)             \
  V(I32_SLE, 0x50)             \
  V(I32_ULT, 0x51)             \
  V(I32_ULE, 0x52)             \
  V(I32_SGT, 0x53)             \
  V(I32_SGE, 0x54)             \
  V(I32_UGT, 0x55)             \
  V(I32_UGE, 0x56)             \
  V(I32_CLZ, 0x57)             \
  V(I32_CTZ, 0x58)             \
  V(I32_POPCNT, 0x59)          \
  V(I32_NOT, 0x5a)             \
  V(I64_ADD, 0x5b)             \
  V(I64_SUB, 0x5c)             \
  V(I64_MUL, 0x5d)             \
  V(I64_SDIV, 0x5e)            \
  V(I64_UDIV, 0x5f)            \
  V(I64_SREM, 0x60)            \
  V(I64_UREM, 0x61)            \
  V(I64_AND, 0x62)             \
  V(I64_OR, 0x63)              \
  V(I64_XOR, 0x64)             \
  V(I64_SHL, 0x65)             \
  V(I64_SHR, 0x66)             \
  V(I64_SAR, 0x67)             \
  V(I64_EQ, 0x68)              \
  V(I64_NE, 0x69)              \
  V(I64_SLT, 0x6a)             \
  V(I64_SLE, 0x6b)             \
  V(I64_ULT, 0x6c)             \
  V(I64_ULE, 0x6d)             \
  V(I64_SGT, 0x6e)             \
  V(I64_SGE, 0x6f)             \
  V(I64_UGT, 0x70)             \
  V(I64_UGE, 0x71)             \
  V(I64_CLZ, 0x72)             \
  V(I64_CTZ, 0x73)             \
  V(I64_POPCNT, 0x74)          \
  V(F32_ADD, 0x75)             \
  V(F32_SUB, 0x76)             \
  V(F32_MUL, 0x77)             \
  V(F32_DIV, 0x78)             \
  V(F32_MIN, 0x79)             \
  V(F32_MAX, 0x7a)             \
  V(F32_ABS, 0x7b)             \
  V(F32_NEG, 0x7c)             \
  V(F32_COPYSIGN, 0x7d)        \
  V(F32_CEIL, 0x7e)            \
  V(F32_FLOOR, 0x7f)           \
  V(F32_TRUNC, 0x80)           \
  V(F32_NEAREST, 0x81)         \
  V(F32_SQRT, 0x82)            \
  V(F32_EQ, 0x83)              \
  V(F32_NE, 0x84)              \
  V(F32_LT, 0x85)              \
  V(F32_LE, 0x86)              \
  V(F32_GT, 0x87)              \
  V(F32_GE, 0x88)              \
  V(F64_ADD, 0x89)             \
  V(F64_SUB, 0x8a)             \
  V(F64_MUL, 0x8b)             \
  V(F64_DIV, 0x8c)             \
  V(F64_MIN, 0x8d)             \
  V(F64_MAX, 0x8e)             \
  V(F64_ABS, 0x8f)             \
  V(F64_NEG, 0x90)             \
  V(F64_COPYSIGN, 0x91)        \
  V(F64_CEIL, 0x92)            \
  V(F64_FLOOR, 0x93)           \
  V(F64_TRUNC, 0x94)           \
  V(F64_NEAREST, 0x95)         \
  V(F64_SQRT, 0x96)            \
  V(F64_EQ, 0x97)              \
  V(F64_NE, 0x98)              \
  V(F64_LT, 0x99)              \
  V(F64_LE, 0x9a)              \
  V(F64_GT, 0x9b)              \
  V(F64_GE, 0x9c)              \
  V(I32_SCONVERT_F32, 0x9d)    \
  V(I32_SCONVERT_F64, 0x9e)    \
  V(I32_UCONVERT_F32, 0x9f)    \
  V(I32_UCONVERT_F64, 0xa0)    \
  V(I32_CONVERT_I64, 0xa1)     \
  V(I64_SCONVERT_F32, 0xa2)    \
  V(I64_SCONVERT_F64, 0xa3)    \
  V(I64_UCONVERT_F32, 0xa4)    \
  V(I64_UCONVERT_F64, 0xa5)    \
  V(I64_SCONVERT_I32, 0xa6)    \
  V(I64_UCONVERT_I32, 0xa7)    \
  V(F32_SCONVERT_I32, 0xa8)    \
  V(F32_UCONVERT_I32, 0xa9)    \
  V(F32_SCONVERT_I64, 0xaa)    \
  V(F32_UCONVERT_I64, 0xab)    \
  V(F32_CONVERT_F64, 0xac)     \
  V(F32_REINTERPRET_I32, 0xad) \
  V(F64_SCONVERT_I32, 0xae)    \
  V(F64_UCONVERT_I32, 0xaf)    \
  V(F64_SCONVERT_I64, 0xb0)    \
  V(F64_UCONVERT_I64, 0xb1)    \
  V(F64_CONVERT_F32, 0xb2)     \
  V(F64_REINTERPRET_I64, 0xb3)

typedef enum Opcode {
  OPCODE_INVALID = -1,
#define OPCODE(name, code) OPCODE_##name = code,
  OPCODES(OPCODE)
#undef OPCODE
} Opcode;

#include "hash.h"

typedef struct OpInfo OpInfo;

typedef uint8_t MemAccess;

typedef enum TokenType {
  TOKEN_TYPE_EOF,
  TOKEN_TYPE_OPEN_PAREN,
  TOKEN_TYPE_CLOSE_PAREN,
  TOKEN_TYPE_ATOM,
  TOKEN_TYPE_STRING,
} TokenType;

typedef struct SourceLocation {
  const char* pos;
  int line;
  int col;
} SourceLocation;

typedef struct SourceRange {
  SourceLocation start;
  SourceLocation end;
} SourceRange;

typedef struct Token {
  TokenType type;
  SourceRange range;
} Token;

typedef struct Source {
  const char* start;
  const char* end;
} Source;

typedef struct Tokenizer {
  Source source;
  SourceLocation loc;
} Tokenizer;

typedef struct Binding {
  char* name;
  size_t offset; /* offset in the output buffer (function bindings skip the
                    signature */
  union {
    Type type;
    struct {
      Type* result_types;
      struct Binding* locals; /* Includes args, they're at the start */
      struct Binding* labels; /* TODO(binji): hack, shouldn't be Binding */
      int num_results;
      int num_args;
      int num_locals; /* Total size of the locals array above, including args */
      int num_labels;
    };
  };
} Binding;

typedef Binding Function;

typedef struct Export {
  char* name;
  int index;
} Export;

typedef struct Module {
  Function* functions;
  Binding* globals;
  Export* exports;
  int num_functions;
  int num_globals;
  int num_exports;
} Module;

typedef struct OutputBuffer {
  void* start;
  size_t size;
  size_t capacity;
} OutputBuffer;

typedef struct NameTypePair {
  const char* name;
  Type type;
} NameTypePair;

static int g_verbose;
static const char* g_filename;
static int g_dump_module;

static const char* s_opcode_names[] = {
#define OPCODE(name, code) [code] = "OPCODE_" #name,
    OPCODES(OPCODE)
#undef OPCODE
};

static NameTypePair s_types[] = {
    {"i32", TYPE_I32},
    {"i64", TYPE_I64},
    {"f32", TYPE_F32},
    {"f64", TYPE_F64},
};

static const char* s_type_names[] = {
    "void", "i32", "i64", "f32", "f64",
};
STATIC_ASSERT(ARRAY_SIZE(s_type_names) == NUM_TYPES);

static void realloc_list(void** elts, int* num_elts, int elt_size) {
  (*num_elts)++;
  int new_size = *num_elts * elt_size;
  *elts = realloc(*elts, new_size);
  if (*elts == NULL) {
    FATAL("unable to alloc %d bytes", new_size);
  }
}

static void dump_memory(const void* start,
                        size_t size,
                        size_t offset,
                        int print_chars,
                        const char* desc) {
  /* mimic xxd output */
  const uint8_t* p = start;
  const uint8_t* end = p + size;
  while (p < end) {
    const uint8_t* line = p;
    const uint8_t* line_end = p + DUMP_OCTETS_PER_LINE;
    printf("%07x: ", (int)((void*)p - start + offset));
    while (p < line_end) {
      int i;
      for (i = 0; i < DUMP_OCTETS_PER_GROUP; ++i, ++p) {
        if (p < end) {
          printf("%02x", *p);
        } else {
          putchar(' ');
          putchar(' ');
        }
      }
      putchar(' ');
    }

    putchar(' ');
    p = line;
    int i;
    for (i = 0; i < DUMP_OCTETS_PER_LINE && p < end; ++i, ++p)
      if (print_chars)
        printf("%c", isprint(*p) ? *p : '.');
    if (desc)
      printf("  ; %s", desc);
    putchar('\n');
  }
}

static void init_output_buffer(OutputBuffer* buf, size_t initial_capacity) {
  buf->start = malloc(initial_capacity);
  if (!buf->start)
    FATAL("unable to allocate %zd bytes\n", initial_capacity);
  buf->size = 0;
  buf->capacity = initial_capacity;
}

static size_t out_data(OutputBuffer* buf,
                       size_t offset,
                       const void* src,
                       size_t size,
                       const char* desc) {
  assert(offset <= buf->size);
  if (offset + size > buf->capacity) {
    size_t new_capacity = buf->capacity * 2;
    buf->start = realloc(buf->start, new_capacity);
    if (!buf->start)
      FATAL("unable to allocate %zd bytes\n", new_capacity);
    buf->capacity = new_capacity;
  }
  memcpy(buf->start + offset, src, size);
  if (g_verbose)
    dump_memory(buf->start + offset, size, offset, 0, desc);
  return offset + size;
}

/* TODO(binji): endianness */
#define OUT_TYPE(name, ctype)                                                \
  static void out_##name(OutputBuffer* buf, ctype value, const char* desc) { \
    buf->size = out_data(buf, buf->size, &value, sizeof(value), desc);       \
  }

OUT_TYPE(u8, uint8_t)
OUT_TYPE(u16, uint16_t)
OUT_TYPE(u32, uint32_t)
OUT_TYPE(u64, uint64_t)
OUT_TYPE(f32, float)
OUT_TYPE(f64, double)

#undef OUT_TYPE

#define OUT_AT_TYPE(name, ctype)                                               \
  static void out_##name##_at(OutputBuffer* buf, uint32_t offset, ctype value, \
                              const char* desc) {                              \
    out_data(buf, offset, &value, sizeof(value), desc);                        \
  }

OUT_AT_TYPE(u8, uint8_t)
OUT_AT_TYPE(u32, uint32_t)

#undef OUT_AT_TYPE

static void out_opcode(OutputBuffer* buf, Opcode opcode) {
  out_u8(buf, (uint8_t)opcode, s_opcode_names[opcode]);
}

static void out_leb128(OutputBuffer* buf, uint32_t value, const char* desc) {
  uint8_t data[5]; /* max 5 bytes */
  int i = 0;
  do {
    assert(i < 5);
    data[i] = value & 0x7f;
    value >>= 7;
    if (value)
      data[i] |= 0x80;
    ++i;
  } while (value);
  buf->size = out_data(buf, buf->size, data, i, desc);
}

static void out_cstr(OutputBuffer* buf, const char* s, const char* desc) {
  buf->size = out_data(buf, buf->size, s, strlen(s) + 1, desc);
}

static void dump_output_buffer(OutputBuffer* buf) {
  dump_memory(buf->start, buf->size, 0, 1, NULL);
}

static void destroy_output_buffer(OutputBuffer* buf) {
  free(buf->start);
}

static int get_binding_by_name(Binding* bindings,
                               int num_bindings,
                               const char* name) {
  int i;
  for (i = 0; i < num_bindings; ++i) {
    Binding* binding = &bindings[i];
    if (binding->name && strcmp(name, binding->name) == 0)
      return i;
  }
  return -1;
}

static int get_function_by_name(Module* module, const char* name) {
  return get_binding_by_name(module->functions, module->num_functions, name);
}

static const OpInfo* get_op_info(Token t) {
  return in_word_set(t.range.start.pos,
                     (int)(t.range.end.pos - t.range.start.pos));
}

static int read_uint32(const char** s, const char* end, uint32_t* out) {
  errno = 0;
  char* endptr;
  uint64_t value = strtoul(*s, &endptr, 10);
  if (endptr != end || errno != 0 || value >= (uint64_t)UINT32_MAX + 1)
    return 0;
  *out = value;
  *s = endptr;
  return 1;
}

static int read_uint64(const char** s, const char* end, uint64_t* out) {
  errno = 0;
  char* endptr;
  *out = strtoull(*s, &endptr, 10);
  if (endptr != end || errno != 0)
    return 0;
  *s = endptr;
  return 1;
}

static int read_double(const char** s, const char* end, double* out) {
  errno = 0;
  char* endptr;
  *out = strtod(*s, &endptr);
  if (endptr != end || errno != 0)
    return 0;
  *s = endptr;
  return 1;
}

static Token read_token(Tokenizer* t) {
  Token result;

  while (t->loc.pos < t->source.end) {
    switch (*t->loc.pos) {
      case ' ':
        t->loc.col++;
        t->loc.pos++;
        break;

      case '\t':
        t->loc.col += TABS_TO_SPACES;
        t->loc.pos++;
        break;

      case '\n':
        t->loc.line++;
        t->loc.col = 1;
        t->loc.pos++;
        break;

      case '"':
        result.type = TOKEN_TYPE_STRING;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;

        while (t->loc.pos < t->source.end) {
          switch (*t->loc.pos) {
            case '\\':
              if (t->loc.pos + 1 < t->source.end) {
                t->loc.col++;
                t->loc.pos++;
                if (*t->loc.pos == 'n' || *t->loc.pos == 't' ||
                    *t->loc.pos == '\\' || *t->loc.pos == '\'' ||
                    *t->loc.pos == '"') {
                  /* newline or tab */
                } else if (isxdigit(*t->loc.pos)) {
                  /* \xx for arbitrary value */
                  if (t->loc.pos + 1 < t->source.end) {
                    t->loc.col++;
                    t->loc.pos++;
                    if (!isxdigit(*t->loc.pos)) {
                      FATAL("%d:%d: bad \\xx escape sequence\n", t->loc.line,
                            t->loc.col);
                    }
                  } else {
                    FATAL("%d:%d: eof in \\xx escape sequence\n", t->loc.line,
                          t->loc.col);
                  }
                } else {
                  FATAL("%d:%d: bad escape sequence\n", t->loc.line,
                        t->loc.col);
                }
              } else {
                FATAL("%d:%d: eof in escape sequence\n", t->loc.line,
                      t->loc.col);
              }
              break;

            case '\n':
              FATAL("%d:%d: newline in string\n", t->loc.line, t->loc.col);
              break;

            case '"':
              t->loc.col++;
              t->loc.pos++;
              goto done_string;
          }

          t->loc.col++;
          t->loc.pos++;
        }
      done_string:
        result.range.end = t->loc;
        return result;

      case ';':
        if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ';') {
          /* line comment */
          while (t->loc.pos < t->source.end) {
            if (*t->loc.pos == '\n')
              break;

            t->loc.col++;
            t->loc.pos++;
          }
        }
        break;

      case '(':
        if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ';') {
          int nesting = 1;
          t->loc.pos += 2;
          t->loc.col += 2;
          while (nesting > 0 && t->loc.pos < t->source.end) {
            switch (*t->loc.pos) {
              case ';':
                if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ')') {
                  nesting--;
                  t->loc.col++;
                  t->loc.pos++;
                }
                t->loc.col++;
                t->loc.pos++;
                break;

              case '(':
                if (t->loc.pos + 1 < t->source.end && t->loc.pos[1] == ';') {
                  nesting++;
                  t->loc.col++;
                  t->loc.pos++;
                }
                t->loc.col++;
                t->loc.pos++;
                break;

              case '\t':
                t->loc.col += TABS_TO_SPACES;
                t->loc.pos++;
                break;

              case '\n':
                t->loc.line++;
                t->loc.col = 1;
                t->loc.pos++;
                break;

              default:
                t->loc.pos++;
                t->loc.col++;
                break;
            }
          }
          break;
        } else {
          result.type = TOKEN_TYPE_OPEN_PAREN;
          result.range.start = t->loc;
          t->loc.col++;
          t->loc.pos++;
          result.range.end = t->loc;
          return result;
        }
        break;

      case ')':
        result.type = TOKEN_TYPE_CLOSE_PAREN;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;
        result.range.end = t->loc;
        return result;

      default:
        result.type = TOKEN_TYPE_ATOM;
        result.range.start = t->loc;
        t->loc.col++;
        t->loc.pos++;
        while (t->loc.pos < t->source.end) {
          switch (*t->loc.pos) {
            case ' ':
            case '\t':
            case '\n':
            case '(':
            case ')':
              result.range.end = t->loc;
              return result;

            default:
              t->loc.col++;
              t->loc.pos++;
              break;
          }
        }
        break;
    }
  }

  result.type = TOKEN_TYPE_EOF;
  result.range.start = t->loc;
  result.range.end = t->loc;
  return result;
}

static void rewind_token(Tokenizer* tokenizer, Token t) {
  tokenizer->loc = t.range.start;
}

static int hexdigit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a';
  } else {
    assert(c >= 'A' && c <= 'F');
    return c - 'A';
  }
}

static char* dup_string_contents(Token t) {
  assert(t.type == TOKEN_TYPE_STRING);
  const char* src = t.range.start.pos;
  const char* end = t.range.end.pos;
  assert(*src == '"' && end[-1] == '"');
  src++;
  end--;
  /* Always allocate enough space for the entire string including the escape
   * characters. It will only get shorter, and this way we only have to iterate
   * through the string once. */
  char* result = malloc(end - src + 1); /* +1 for \0 */
  char* dest = result;
  while (src < end) {
    if (*src == '\\') {
      src++;
      switch (*src) {
        case 'n':
          *dest++ = '\n';
          break;
        case 't':
          *dest++ = '\t';
          break;
        case '\\':
          *dest++ = '\\';
          break;
        case '\'':
          *dest++ = '\'';
          break;
        case '\"':
          *dest++ = '\"';
          break;
        default:
          /* The string should be validated already, so we know this is a hex
           * sequence */
          *dest++ = (hexdigit(src[0]) << 4) | hexdigit(src[1]);
          src++;
          break;
      }
      src++;
    } else {
      *dest++ = *src++;
    }
  }
  *dest++ = '\0';
  return result;
}

static void expect_open(Token t) {
  if (t.type != TOKEN_TYPE_OPEN_PAREN) {
    FATAL("%d:%d: expected '(', not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_close(Token t) {
  if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    FATAL("%d:%d: expected ')', not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_atom(Token t) {
  if (t.type != TOKEN_TYPE_ATOM) {
    FATAL("%d:%d: expected ATOM, not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_string(Token t) {
  if (t.type != TOKEN_TYPE_STRING) {
    FATAL("%d:%d: expected STRING, not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void expect_var_name(Token t) {
  if (t.type != TOKEN_TYPE_ATOM) {
    FATAL("%d:%d: expected ATOM, not \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }

  if (t.range.end.pos - t.range.start.pos < 1 || t.range.start.pos[0] != '$') {
    FATAL("%d:%d: expected name to begin with $\n", t.range.start.line,
          t.range.start.col);
  }
}

static void check_opcode(SourceLocation loc, Opcode opcode) {
  if (opcode == OPCODE_INVALID)
    FATAL("%d:%d: no opcode for instruction\n", loc.line, loc.col);
}

static int match_atom(Token t, const char* s) {
  return strncmp(t.range.start.pos, s, t.range.end.pos - t.range.start.pos) ==
         0;
}

static int match_type(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_types); ++i) {
    if (match_atom(t, s_types[i].name)) {
      if (type)
        *type = s_types[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_load_store_aligned(Token t, OpInfo* op_info) {
  /* look for a slash near the end of the string */
  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos - 1;
  while (end > p) {
    if (*end == '/') {
      const OpInfo* found = in_word_set(p, (int)(end - p));
      if (!(found && (found->op_type == OP_LOAD || found->op_type == OP_STORE)))
        return 0;

      ++end;
      uint32_t alignment;
      if (!read_uint32(&end, t.range.end.pos, &alignment))
        return 0;

      /* check that alignment is power-of-two */
      if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        FATAL("%d:%d: alignment must be power-of-two\n", t.range.start.line,
              t.range.start.col);
      }

      int native_mem_type_alignment[] = {
          1, /* MEM_TYPE_I8 */
          2, /* MEM_TYPE_I16 */
          4, /* MEM_TYPE_I32 */
          8, /* MEM_TYPE_I64 */
          4, /* MEM_TYPE_F32 */
          8, /* MEM_TYPE_F64 */
      };

      *op_info = *found;
      if (alignment && alignment < native_mem_type_alignment[op_info->type2])
        op_info->access |= 0x8; /* unaligned access */
      return 1;
    }
    end--;
  }
  return 0;
}

static void unexpected_token(Token t) {
  if (t.type == TOKEN_TYPE_EOF) {
    FATAL("%d:%d: unexpected EOF\n", t.range.start.line, t.range.start.col);
  } else {
    FATAL("%d:%d: unexpected token \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  }
}

static void check_type(SourceLocation loc,
                       Type actual,
                       Type expected,
                       const char* desc) {
  if (actual != expected) {
    FATAL("%d:%d: type mismatch%s. got %s, expected %s\n", loc.line, loc.col,
          desc, s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_arg(SourceLocation loc,
                           Type actual,
                           Type expected,
                           const char* desc,
                           int arg_index) {
  if (actual != expected) {
    FATAL("%d:%d: type mismatch for argument %d of %s. got %s, expected %s\n",
          loc.line, loc.col, arg_index, desc, s_type_names[actual],
          s_type_names[expected]);
  }
}

static Type get_result_type(SourceLocation loc, Function* function) {
  switch (function->num_results) {
    case 0:
      return TYPE_VOID;

    case 1:
      return function->result_types[0];

    default:
      FATAL("%d:%d: multiple return values currently unsupported\n", loc.line,
            loc.col);
      return TYPE_VOID;
  }
}

static void parse_generic(Tokenizer* tokenizer) {
  int nesting = 1;
  while (nesting > 0) {
    Token t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      nesting++;
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      nesting--;
    } else if (t.type == TOKEN_TYPE_EOF) {
      FATAL("unexpected eof.\n");
    }
  }
}

static int parse_var(Tokenizer* tokenizer,
                     Binding* bindings,
                     int num_bindings,
                     const char* desc) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    const char* p = t.range.start.pos;
    const char* end = t.range.end.pos;
    if (end - p >= 1 && p[0] == '$') {
      /* var name */
      int i;
      for (i = 0; i < num_bindings; ++i) {
        const char* name = bindings[i].name;
        if (name && strncmp(name, p, end - p) == 0)
          return i;
      }
      FATAL("%d:%d: undefined %s variable \"%.*s\"\n", t.range.start.line,
            t.range.start.col, desc, (int)(end - p), p);
    } else {
      /* var index */
      uint32_t index;
      if (!read_uint32(&p, t.range.end.pos, &index) || p != t.range.end.pos) {
        FATAL("%d:%d: invalid var index\n", t.range.start.line,
              t.range.start.col);
      }

      if (index >= num_bindings) {
        FATAL("%d:%d: %s variable out of range (max %d)\n", t.range.start.line,
              t.range.start.col, desc, num_bindings);
      }

      return index;
    }
  } else {
    unexpected_token(t);
    return -1;
  }
}

static int parse_function_var(Tokenizer* tokenizer, Module* module) {
  return parse_var(tokenizer, module->functions, module->num_functions,
                   "function");
}

static int parse_global_var(Tokenizer* tokenizer, Module* module) {
  return parse_var(tokenizer, module->globals, module->num_globals, "global");
}

static int parse_local_var(Tokenizer* tokenizer, Function* function) {
  return parse_var(tokenizer, function->locals, function->num_locals, "local");
}

static int parse_label_var(Tokenizer* tokenizer, Function* function) {
  return parse_var(tokenizer, function->labels, function->num_labels, "label");
}

static void parse_type(Tokenizer* tokenizer, Type* type) {
  Token t = read_token(tokenizer);
  if (!match_type(t, type)) {
    FATAL("%d:%d: expected type\n", t.range.start.line, t.range.start.col);
  }
}

static Type parse_expr(Tokenizer* tokenizer,
                       Module* module,
                       Function* function,
                       OutputBuffer* buf);

static Type parse_block(Tokenizer* tokenizer,
                        Module* module,
                        Function* function,
                        OutputBuffer* buf) {
  size_t offset = buf->size;
  out_u8(buf, 0, "num expressions");
  int num_exprs = 0;
  Type type;
  while (1) {
    type = parse_expr(tokenizer, module, function, buf);
    ++num_exprs;
    Token t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_CLOSE_PAREN)
      break;
    rewind_token(tokenizer, t);
  }
  out_u8_at(buf, offset, num_exprs, "FIXUP num expressions");
  return type;
}

static void parse_literal(Tokenizer* tokenizer, OutputBuffer* buf, Type type) {
  Token t = read_token(tokenizer);
  expect_atom(t);
  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  switch (type) {
    case TYPE_I32: {
      uint32_t value;
      if (!read_uint32(&p, end, &value)) {
        FATAL("%d:%d: invalid unsigned 32-bit int\n", t.range.start.line,
              t.range.start.col);
      }
      out_u32(buf, value, "u32 literal");
      break;
    }

    case TYPE_I64: {
      uint64_t value;
      if (!read_uint64(&p, end, &value)) {
        FATAL("%d:%d: invalid unsigned 64-bit int\n", t.range.start.line,
              t.range.start.col);
      }
      out_u64(buf, value, "u64 literal");
      break;
    }

    case TYPE_F32:
    case TYPE_F64: {
      double value;
      if (!read_double(&p, end, &value)) {
        FATAL("%d:%d: invalid double\n", t.range.start.line, t.range.start.col);
      }
      if (type == TYPE_F32)
        out_f32(buf, (float)value, "f32 literal");
      else
        out_f64(buf, value, "f64 literal");
      break;
    }

    default:
      assert(0);
  }
}

static Type parse_switch(Tokenizer* tokenizer,
                         Module* module,
                         Function* function,
                         OutputBuffer* buf,
                         Type in_type) {
  int num_cases = 0;
  Type cond_type = parse_expr(tokenizer, module, function, buf);
  check_type(tokenizer->loc, cond_type, in_type, " in switch condition");
  Type type = TYPE_VOID;
  Token t = read_token(tokenizer);
  while (1) {
    expect_open(t);
    Token open = t;
    t = read_token(tokenizer);
    expect_atom(t);
    if (match_atom(t, "case")) {
      parse_literal(tokenizer, buf, in_type);
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        Type value_type;
        while (1) {
          value_type = parse_expr(tokenizer, module, function, buf);
          t = read_token(tokenizer);
          if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
            break;
          } else if (t.type == TOKEN_TYPE_ATOM &&
                     match_atom(t, "fallthrough")) {
            expect_close(read_token(tokenizer));
            break;
          }
          rewind_token(tokenizer, t);
        }

        if (++num_cases == 1) {
          type = value_type;
        } else if (value_type == TYPE_VOID) {
          type = TYPE_VOID;
        } else {
          check_type(t.range.start, value_type, type, " in switch case");
        }
      }
    } else {
      /* default case */
      rewind_token(tokenizer, open);
      Type value_type = parse_expr(tokenizer, module, function, buf);
      if (value_type == TYPE_VOID) {
        type = TYPE_VOID;
      } else {
        check_type(t.range.start, value_type, type, " in switch default case");
      }
      expect_close(read_token(tokenizer));
      break;
    }
    t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_CLOSE_PAREN)
      break;
  }
  return type;
}

static Type parse_expr(Tokenizer* tokenizer,
                       Module* module,
                       Function* function,
                       OutputBuffer* buf) {
  Type type = TYPE_VOID;
  expect_open(read_token(tokenizer));
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    OpInfo op_info_aligned;
    const OpInfo* op_info = get_op_info(t);
    if (!op_info) {
      if (!match_load_store_aligned(t, &op_info_aligned))
        unexpected_token(t);
      op_info = &op_info_aligned;
    }
    switch (op_info->op_type) {
      case OP_BINARY: {
        check_opcode(t.range.start, op_info->opcode);
        out_opcode(buf, op_info->opcode);
        Type value0_type = parse_expr(tokenizer, module, function, buf);
        check_type_arg(t.range.start, value0_type, op_info->type, "binary op",
                       0);
        Type value1_type = parse_expr(tokenizer, module, function, buf);
        check_type_arg(t.range.start, value1_type, op_info->type, "binary op",
                       1);
        assert(value0_type == value1_type);
        type = value0_type;
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_BLOCK:
        out_opcode(buf, OPCODE_BLOCK);
        type = parse_block(tokenizer, module, function, buf);
        break;

      case OP_BREAK:
        /* opcode = OPCODE_BREAK; */
        t = read_token(tokenizer);
        if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
          rewind_token(tokenizer, t);
          /* TODO(binji): how to check that the given label is a parent? */
          parse_label_var(tokenizer, function);
          expect_close(read_token(tokenizer));
        }
        break;

      case OP_CALL: {
        out_opcode(buf, OPCODE_CALL);
        int index = parse_function_var(tokenizer, module);
        out_leb128(buf, index, "func index");
        Function* callee = &module->functions[index];

        int num_args = 0;
        while (1) {
          Token t = read_token(tokenizer);
          if (t.type == TOKEN_TYPE_CLOSE_PAREN)
            break;
          rewind_token(tokenizer, t);
          if (++num_args > callee->num_args) {
            FATAL(
                "%d:%d: too many arguments to function. got %d, expected %d\n",
                t.range.start.line, t.range.start.col, num_args,
                callee->num_args);
          }
          Type arg_type = parse_expr(tokenizer, module, function, buf);
          Type expected = callee->locals[num_args - 1].type;
          check_type_arg(t.range.start, arg_type, expected, "call",
                         num_args - 1);
        }

        if (num_args < callee->num_args) {
          FATAL("%d:%d: too few arguments to function. got %d, expected %d\n",
                t.range.start.line, t.range.start.col, num_args,
                callee->num_args);
        }

        type = get_result_type(t.range.start, callee);
        break;
      }

      case OP_CALL_INDIRECT:
        /* TODO(binji) */
        break;

      case OP_COMPARE: {
        check_opcode(t.range.start, op_info->opcode);
        out_opcode(buf, op_info->opcode);
        Type value0_type = parse_expr(tokenizer, module, function, buf);
        check_type_arg(t.range.start, value0_type, op_info->type, "compare op",
                       0);
        Type value1_type = parse_expr(tokenizer, module, function, buf);
        check_type_arg(t.range.start, value1_type, op_info->type, "compare op",
                       1);
        type = TYPE_I32;
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_CONST:
        check_opcode(t.range.start, op_info->opcode);
        out_opcode(buf, op_info->opcode);
        parse_literal(tokenizer, buf, op_info->type);
        expect_close(read_token(tokenizer));
        type = op_info->type;
        break;

      case OP_CONVERT: {
        check_opcode(t.range.start, op_info->opcode);
        out_opcode(buf, op_info->opcode);
        Type value_type = parse_expr(tokenizer, module, function, buf);
        check_type(t.range.start, value_type, op_info->type2, " of convert op");
        expect_close(read_token(tokenizer));
        type = op_info->type;
        break;
      }

      case OP_DESTRUCT:
        /* TODO(binji) */
        break;

      case OP_GET_LOCAL: {
        /* opcode = OPCODE_GET_LOCAL; */
        int index = parse_local_var(tokenizer, function);
        type = function->locals[index].type;
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_IF: {
        uint32_t opcode_offset = buf->size;
        out_opcode(buf, OPCODE_IF);
        Type cond_type = parse_expr(tokenizer, module, function, buf);
        check_type(tokenizer->loc, cond_type, TYPE_I32, " of condition");
        Type true_type = parse_expr(tokenizer, module, function, buf);
        t = read_token(tokenizer);
        if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
          out_u8_at(buf, opcode_offset, OPCODE_IF_THEN, "FIXUP OPCODE_IF_THEN");
          rewind_token(tokenizer, t);
          Type false_type = parse_expr(tokenizer, module, function, buf);
          if (true_type == TYPE_VOID || false_type == TYPE_VOID) {
            type = TYPE_VOID;
          } else {
            check_type(tokenizer->loc, false_type, true_type,
                       " between true and false branches");
            type = true_type;
          }
          expect_close(read_token(tokenizer));
        } else {
          type = true_type;
        }
        break;
      }

      case OP_LABEL: {
        t = read_token(tokenizer);
        realloc_list((void**)&function->labels, &function->num_labels,
                     sizeof(Binding));
        Binding* binding = &function->labels[function->num_labels - 1];
        binding->name = NULL;

        if (t.type == TOKEN_TYPE_ATOM) {
          /* label */
          expect_var_name(t);
          binding->name =
              strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
        } else if (t.type == TOKEN_TYPE_OPEN_PAREN) {
          /* no label */
          rewind_token(tokenizer, t);
        } else {
          unexpected_token(t);
        }
        type = parse_block(tokenizer, module, function, buf);
        break;
      }

      case OP_LOAD: {
        check_opcode(t.range.start, op_info->opcode);
        out_opcode(buf, op_info->opcode);
        out_u8(buf, op_info->access, "load access byte");
        Type index_type = parse_expr(tokenizer, module, function, buf);
        check_type(t.range.start, index_type, TYPE_I32, " of load index");
        expect_close(read_token(tokenizer));
        type = op_info->type;
        break;
      }

      case OP_LOAD_GLOBAL: {
        out_opcode(buf, OPCODE_GET_GLOBAL);
        int index = parse_global_var(tokenizer, module);
        out_leb128(buf, index, "global index");
        type = module->globals[index].type;
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_LOOP:
        out_opcode(buf, OPCODE_LOOP);
        type = parse_block(tokenizer, module, function, buf);
        break;

      case OP_NOP:
        out_opcode(buf, OPCODE_NOP);
        expect_close(read_token(tokenizer));
        break;

      case OP_RETURN: {
        out_opcode(buf, OPCODE_RETURN);
        int num_results = 0;
        while (1) {
          t = read_token(tokenizer);
          if (t.type == TOKEN_TYPE_CLOSE_PAREN)
            break;
          if (++num_results > function->num_results) {
            FATAL("%d:%d: too many return values. got %d, expected %d\n",
                  t.range.start.line, t.range.start.col, num_results,
                  function->num_results);
          }
          rewind_token(tokenizer, t);
          Type result_type = parse_expr(tokenizer, module, function, buf);
          Type expected = function->result_types[num_results - 1];
          check_type_arg(t.range.start, result_type, expected, "return",
                         num_results - 1);
        }

        if (num_results < function->num_results) {
          FATAL("%d:%d: too few return values. got %d, expected %d\n",
                t.range.start.line, t.range.start.col, num_results,
                function->num_results);
        }

        type = get_result_type(t.range.start, function);
        break;
      }

      case OP_SET_LOCAL: {
        /* opcode = OPCODE_SET_LOCAL; */
        int index = parse_local_var(tokenizer, function);
        Binding* binding = &function->locals[index];
        type = parse_expr(tokenizer, module, function, buf);
        check_type(t.range.start, type, binding->type, "");
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_STORE: {
        check_opcode(t.range.start, op_info->opcode);
        out_opcode(buf, op_info->opcode);
        out_u8(buf, op_info->access, "store access byte");
        Type index_type = parse_expr(tokenizer, module, function, buf);
        check_type(t.range.start, index_type, TYPE_I32, " of store index");
        Type value_type = parse_expr(tokenizer, module, function, buf);
        check_type(t.range.start, value_type, op_info->type, " of store value");
        expect_close(read_token(tokenizer));
        type = op_info->type;
        break;
      }

      case OP_STORE_GLOBAL: {
        out_opcode(buf, OPCODE_SET_GLOBAL);
        int index = parse_global_var(tokenizer, module);
        out_leb128(buf, index, "global index");
        Binding* binding = &module->globals[index];
        type = parse_expr(tokenizer, module, function, buf);
        check_type(t.range.start, type, binding->type, "");
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_SWITCH:
        check_opcode(t.range.start, op_info->opcode);
        type = parse_switch(tokenizer, module, function, buf, op_info->type);
        break;

      case OP_UNARY: {
        check_opcode(t.range.start, op_info->opcode);
        out_opcode(buf, op_info->opcode);
        Type value_type = parse_expr(tokenizer, module, function, buf);
        check_type(t.range.start, value_type, op_info->type, " of unary op");
        type = value_type;
        expect_close(read_token(tokenizer));
        break;
      }

      default:
        assert(0);
        break;
    }
  }
  return type;
}

static void parse_func(Tokenizer* tokenizer,
                       Module* module,
                       Function* function,
                       OutputBuffer* buf) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    t = read_token(tokenizer);
  }

  Type type = TYPE_VOID;
  while (1) {
    if (t.type == TOKEN_TYPE_CLOSE_PAREN)
      break;
    if (t.type != TOKEN_TYPE_OPEN_PAREN)
      unexpected_token(t);

    Token open = t;
    t = read_token(tokenizer);
    if (t.type != TOKEN_TYPE_ATOM)
      unexpected_token(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_PARAM:
      case OP_RESULT:
      case OP_LOCAL:
        /* Skip, this has already been pre-parsed */
        parse_generic(tokenizer);
        break;

      default:
        rewind_token(tokenizer, open);
        type = parse_expr(tokenizer, module, function, buf);
        break;
    }
    t = read_token(tokenizer);
  }

  Type result_type = get_result_type(t.range.start, function);
  if (result_type != TYPE_VOID)
    check_type(t.range.start, type, result_type, " in function result");
}

static void preparse_binding_list(Tokenizer* tokenizer,
                                  Binding** bindings,
                                  int* num_bindings,
                                  const char* desc) {
  Token t = read_token(tokenizer);
  Type type;
  if (match_type(t, &type)) {
    while (1) {
      realloc_list((void**)bindings, num_bindings, sizeof(Binding));
      Binding* binding = &(*bindings)[*num_bindings - 1];
      binding->name = NULL;
      binding->type = type;

      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_CLOSE_PAREN)
        break;
      else if (!match_type(t, &type))
        unexpected_token(t);
    }
  } else {
    expect_var_name(t);
    parse_type(tokenizer, &type);
    expect_close(read_token(tokenizer));

    char* name =
        strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
    if (get_binding_by_name(*bindings, *num_bindings, name) != -1) {
      FATAL("%d:%d: redefinition of %s \"%s\"\n", t.range.start.line,
            t.range.start.col, desc, name);
    }

    realloc_list((void**)bindings, num_bindings, sizeof(Binding));
    Binding* binding = &(*bindings)[*num_bindings - 1];
    binding->name = name;
    binding->type = type;
  }
}

static void preparse_func(Tokenizer* tokenizer, Module* module) {
  realloc_list((void**)&module->functions, &module->num_functions,
               sizeof(Function));
  Function* function = &module->functions[module->num_functions - 1];
  memset(function, 0, sizeof(Function));

  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    char* name =
        strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
    if (get_function_by_name(module, name) != -1) {
      FATAL("%d:%d: redefinition of function \"%s\"\n", t.range.start.line,
            t.range.start.col, name);
    }

    function->name = name;
    t = read_token(tokenizer);
  }

  while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    if (t.type != TOKEN_TYPE_OPEN_PAREN)
      unexpected_token(t);

    t = read_token(tokenizer);
    if (t.type != TOKEN_TYPE_ATOM)
      unexpected_token(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_PARAM:
        /* Don't allow adding params after locals */
        if (function->num_args != function->num_locals) {
          FATAL("%d:%d: parameters must come before locals\n",
                t.range.start.line, t.range.start.col);
        }
        preparse_binding_list(tokenizer, &function->locals, &function->num_args,
                              "function argument");
        function->num_locals = function->num_args;
        break;

      case OP_RESULT: {
        t = read_token(tokenizer);
        while (1) {
          Type type;
          if (!match_type(t, &type))
            unexpected_token(t);
          realloc_list((void**)&function->result_types, &function->num_results,
                       sizeof(Type));
          function->result_types[function->num_results - 1] = type;

          t = read_token(tokenizer);
          if (t.type == TOKEN_TYPE_CLOSE_PAREN)
            break;
        }
        break;
      }

      case OP_LOCAL:
        preparse_binding_list(tokenizer, &function->locals,
                              &function->num_locals, "local");
        break;

      default:
        rewind_token(tokenizer, t);
        parse_generic(tokenizer);
        break;
    }
    t = read_token(tokenizer);
  }
}

static void preparse_module(Tokenizer* tokenizer, Module* module) {
  Token t = read_token(tokenizer);
  Token first = t;
  while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    if (t.type != TOKEN_TYPE_OPEN_PAREN)
      unexpected_token(t);

    t = read_token(tokenizer);
    if (t.type != TOKEN_TYPE_ATOM)
      unexpected_token(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_FUNC:
        preparse_func(tokenizer, module);
        break;

      case OP_GLOBAL:
        preparse_binding_list(tokenizer, &module->globals, &module->num_globals,
                              "global");
        break;

      default:
        parse_generic(tokenizer);
        break;
    }
    t = read_token(tokenizer);
  }
  rewind_token(tokenizer, first);
}

static void destroy_binding_list(Binding* bindings, int num_bindings) {
  int i;
  for (i = 0; i < num_bindings; ++i)
    free(bindings[i].name);
  free(bindings);
}

static void destroy_module(Module* module) {
  int i;
  for (i = 0; i < module->num_functions; ++i) {
    Function* function = &module->functions[i];
    free(function->name);
    free(function->result_types);
    destroy_binding_list(function->locals, function->num_locals);
    destroy_binding_list(function->labels, function->num_labels);
  }
  destroy_binding_list(module->globals, module->num_globals);
  for (i = 0; i < module->num_exports; ++i)
    free(module->exports[i].name);
  free(module->exports);
}

static void out_module_header(OutputBuffer* buf, Module* module) {
  out_u8(buf, DEFAULT_MEMORY_SIZE_LOG2, "mem size log 2");
  out_u8(buf, DEFAULT_MEMORY_EXPORT, "export mem");
  out_u16(buf, module->num_globals, "num globals");
  out_u16(buf, module->num_functions, "num funcs");
  out_u16(buf, 0, "num data segments");

  int i;
  for (i = 0; i < module->num_globals; ++i) {
    Binding* global = &module->globals[i];
    global->offset = buf->size;
    /* TODO(binji): v8-native-prototype globals use mem types, not local types.
       The spec currently specifies local types, and uses an anonymous memory
       space for storage. Resolve this discrepancy. */
    const uint8_t global_type_codes[NUM_TYPES] = {-1, 4, 6, 8, 9};
    out_u32(buf, 0, "global name offset");
    out_u8(buf, global_type_codes[global->type], "global mem type");
    out_u32(buf, 0, "global offset");
    out_u8(buf, 0, "export global");
  }

  for (i = 0; i < module->num_functions; ++i) {
    Function* function = &module->functions[i];

    out_u8(buf, function->num_args, "func num args");
    out_u8(buf, function->num_results ? function->result_types[0] : 0,
           "func result type");
    int j;
    for (j = 0; j < function->num_args; ++j)
      out_u8(buf, function->locals[j].type, "func arg type");
#define CODE_START_OFFSET 4
#define CODE_END_OFFSET 8
#define FUNCTION_EXPORTED_OFFSET 20
    /* function offset skips the signature; it is variable size, and everything
     * we need to fix up is afterward */
    function->offset = buf->size;
    out_u32(buf, 0, "func name offset");
    out_u32(buf, 0, "func code start offset");
    out_u32(buf, 0, "func code end offset");

    int num_locals[NUM_TYPES] = {};
    for (j = function->num_args; j < function->num_locals; ++j)
      num_locals[function->locals[j].type]++;

    out_u16(buf, num_locals[TYPE_I32], "num local i32");
    out_u16(buf, num_locals[TYPE_I64], "num local i64");
    out_u16(buf, num_locals[TYPE_F32], "num local f32");
    out_u16(buf, num_locals[TYPE_F64], "num local f64");
    out_u8(buf, 0, "export func");
    out_u8(buf, 0, "func external");
  }
}

static void out_module_footer(OutputBuffer* buf, Module* module) {
  /* TODO(binji): output data segment */

  /* output name table */
  /* TODO(binji): uniquify names */
  int i;
  for (i = 0; i < module->num_exports; ++i) {
    Export* export = &module->exports[i];
    Function* function = &module->functions[export->index];
    out_u32_at(buf, function->offset, buf->size, "FIXUP func name offset");
    out_cstr(buf, export->name, "export name");
  }
}

static void parse_module(Tokenizer* tokenizer) {
  Module module = {};
  preparse_module(tokenizer, &module);

  OutputBuffer output = {};
  init_output_buffer(&output, INITIAL_OUTPUT_BUFFER_CAPACITY);
  out_module_header(&output, &module);

  int function_index = 0;
  Token t = read_token(tokenizer);
  while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    if (t.type != TOKEN_TYPE_OPEN_PAREN)
      unexpected_token(t);

    t = read_token(tokenizer);
    if (t.type != TOKEN_TYPE_ATOM)
      unexpected_token(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_FUNC: {
        Function* function = &module.functions[function_index++];
        out_u32_at(&output, function->offset + CODE_START_OFFSET, output.size,
                   "FIXUP func code start offset");
        parse_func(tokenizer, &module, function, &output);
        out_u32_at(&output, function->offset + CODE_END_OFFSET, output.size,
                   "FIXUP func code end offset");
        break;
      }

      case OP_GLOBAL:
        parse_generic(tokenizer);
        break;

      case OP_EXPORT: {
        t = read_token(tokenizer);
        expect_string(t);
        int index = parse_function_var(tokenizer, &module);

        realloc_list((void**)&module.exports, &module.num_exports,
                     sizeof(Export));
        Export* export = &module.exports[module.num_exports - 1];
        export->name = dup_string_contents(t);
        export->index = index;

        Function* exported = &module.functions[index];
        out_u32_at(&output, exported->offset + FUNCTION_EXPORTED_OFFSET, 1,
                   "FIXUP func exported");
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_TABLE:
        parse_generic(tokenizer);
        break;

      case OP_MEMORY:
        parse_generic(tokenizer);
        break;

      default:
        unexpected_token(t);
        break;
    }
    t = read_token(tokenizer);
  }

  out_module_footer(&output, &module);

  if (g_dump_module)
    dump_output_buffer(&output);

  destroy_output_buffer(&output);
  destroy_module(&module);
}

static void parse(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  while (t.type != TOKEN_TYPE_EOF) {
    if (t.type != TOKEN_TYPE_OPEN_PAREN)
      unexpected_token(t);

    t = read_token(tokenizer);
    if (t.type != TOKEN_TYPE_ATOM)
      unexpected_token(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_MODULE:
        parse_module(tokenizer);
        break;

      case OP_ASSERT_EQ:
      case OP_ASSERT_INVALID:
      case OP_INVOKE:
        parse_generic(tokenizer);
        break;

      default:
        unexpected_token(t);
        break;
    }
    t = read_token(tokenizer);
  }
}

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  FLAG_DUMP_MODULE,
  NUM_FLAGS
};

static struct option g_long_options[] = {
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {"dump-module", no_argument, NULL, 'd'},
    {NULL, 0, NULL, 0},
};
STATIC_ASSERT(NUM_FLAGS + 1 == ARRAY_SIZE(g_long_options));

typedef struct OptionHelp {
  int flag;
  const char* metavar;
  const char* help;
} OptionHelp;

static OptionHelp g_option_help[] = {
    {FLAG_VERBOSE, NULL, "use multiple times for more info"},
    {FLAG_DUMP_MODULE, NULL, "print a hexdump of the module to stdout"},
    {NUM_FLAGS, NULL},
};

static void usage(const char* prog) {
  printf("usage: %s [option] filename\n", prog);
  printf("options:\n");
  struct option* opt = &g_long_options[0];
  int i = 0;
  for (; opt->name; ++i, ++opt) {
    OptionHelp* help = NULL;

    int n = 0;
    while (g_option_help[n].help) {
      if (i == g_option_help[n].flag) {
        help = &g_option_help[n];
        break;
      }
      n++;
    }

    if (opt->val) {
      printf("  -%c, ", opt->val);
    } else {
      printf("      ");
    }

    if (help && help->metavar) {
      char buf[100];
      snprintf(buf, 100, "%s=%s", opt->name, help->metavar);
      printf("--%-30s", buf);
    } else {
      printf("--%-30s", opt->name);
    }

    if (help) {
      printf("%s", help->help);
    }

    printf("\n");
  }
  exit(0);
}

static void parse_options(int argc, char** argv) {
  int c;
  int option_index;

  while (1) {
    c = getopt_long(argc, argv, "vhd", g_long_options, &option_index);
    if (c == -1) {
      break;
    }

  redo_switch:
    switch (c) {
      case 0:
        c = g_long_options[option_index].val;
        if (c) {
          goto redo_switch;
        }

        switch (option_index) {
          case FLAG_VERBOSE:
          case FLAG_HELP:
          case FLAG_DUMP_MODULE:
            /* Handled above by goto */
            assert(0);
        }
        break;

      case 'v':
        g_verbose++;
        break;

      case 'h':
        usage(argv[0]);

      case 'd':
        g_dump_module = 1;
        break;

      case '?':
        break;

      default:
        FATAL("getopt_long returned '%c' (%d)\n", c, c);
        break;
    }
  }

  if (optind < argc) {
    g_filename = argv[optind];
  } else {
    FATAL("No filename given.\n");
    usage(argv[0]);
  }
}

int main(int argc, char** argv) {
  parse_options(argc, argv);

  FILE* f = fopen(g_filename, "rb");
  if (!f) {
    FATAL("unable to read %s\n", g_filename);
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  void* data = malloc(fsize);
  if (!data) {
    FATAL("unable to alloc %zd bytes\n", fsize);
  }
  if (fread(data, 1, fsize, f) != fsize) {
    FATAL("unable to read %zd bytes from %s\n", fsize, g_filename);
  }
  fclose(f);

  Source source;
  source.start = data;
  source.end = data + fsize;

  Tokenizer tokenizer;
  tokenizer.source = source;
  tokenizer.loc.pos = source.start;
  tokenizer.loc.line = 1;
  tokenizer.loc.col = 1;

  parse(&tokenizer);
  return 0;
}

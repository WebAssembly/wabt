#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm.h"
#include "wasm-parse.h"

#define TABS_TO_SPACES 8
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define FATAL_AT(loc, ...)                           \
  fprintf(stderr, "%d:%d: ", (loc).line, (loc).col), \
      fprintf(stderr, __VA_ARGS__), exit(1)
#define STATIC_ASSERT__(x, c) typedef char static_assert_##c[x ? 1 : -1]
#define STATIC_ASSERT_(x, c) STATIC_ASSERT__(x, c)
#define STATIC_ASSERT(x) STATIC_ASSERT_(x, __COUNTER__)

#define INITIAL_VECTOR_CAPACITY 8
#define INITIAL_OUTPUT_BUFFER_CAPACITY (64 * 1024)
#define DEFAULT_MEMORY_EXPORT 1

#define DUMP_OCTETS_PER_LINE 16
#define DUMP_OCTETS_PER_GROUP 2

#include "hash.h"

typedef struct OpInfo OpInfo;
typedef uint8_t MemAccess;

static void* append_element(void** data,
                            size_t* size,
                            size_t* capacity,
                            size_t elt_size);

#define DEFINE_VECTOR(name, type)                                         \
  void destroy_##name##_vector(type##Vector* vec) { free(vec->data); }    \
  type* append_##name(type##Vector* vec) {                                \
    return append_element((void**)&vec->data, &vec->size, &vec->capacity, \
                          sizeof(type));                                  \
  }

DEFINE_VECTOR(type, Type)
DEFINE_VECTOR(binding, Binding)
DEFINE_VECTOR(variable, Variable)
DEFINE_VECTOR(function, Function)
DEFINE_VECTOR(export, Export)
DEFINE_VECTOR(segment, Segment)

typedef struct OutputBuffer {
  void* start;
  size_t size;
  size_t capacity;
} OutputBuffer;

typedef struct NameTypePair {
  const char* name;
  Type type;
} NameTypePair;

int g_verbose;
const char* g_outfile;
int g_dump_module;

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

static uint32_t log_two_u32(uint32_t x) {
  if (!x)
    return 0;
  return sizeof(unsigned int) * 8 - __builtin_clz(x - 1);
}

void* append_element(void** data,
                     size_t* size,
                     size_t* capacity,
                     size_t elt_size) {
  if (*size + 1 > *capacity) {
    size_t new_capacity = *capacity ? *capacity * 2 : INITIAL_VECTOR_CAPACITY;
    size_t new_size = new_capacity * elt_size;
    *data = realloc(*data, new_size);
    if (*data == NULL)
      FATAL("unable to alloc %zd bytes", new_size);
    *capacity = new_capacity;
  }
  return *data + (*size)++ * elt_size;
}

static void pop_binding(BindingVector* vec) {
  assert(vec->size > 0);
  free(vec->data[vec->size - 1].name);
  vec->size--;
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
    /* if there are multiple lines, only print the desc on the last one */
    if (p >= end && desc)
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

static void ensure_output_buffer_capacity(OutputBuffer* buf,
                                          size_t ensure_capacity) {
  if (ensure_capacity > buf->capacity) {
    size_t new_capacity = buf->capacity * 2;
    while (new_capacity < ensure_capacity)
      new_capacity *= 2;
    buf->start = realloc(buf->start, new_capacity);
    if (!buf->start)
      FATAL("unable to allocate %zd bytes\n", new_capacity);
    buf->capacity = new_capacity;
  }
}

static size_t out_data(OutputBuffer* buf,
                       size_t offset,
                       const void* src,
                       size_t size,
                       const char* desc) {
  assert(offset <= buf->size);
  ensure_output_buffer_capacity(buf, offset + size);
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

static size_t copy_string_contents(Token t, char* dest, size_t size);

static void out_string_token(OutputBuffer* buf, Token t, const char* desc) {
  assert(t.type == TOKEN_TYPE_STRING);
  size_t max_size = t.range.end.pos - t.range.start.pos;
  size_t offset = buf->size;
  ensure_output_buffer_capacity(buf, offset + max_size);
  void* dest = buf->start + offset;
  int actual_size = copy_string_contents(t, dest, max_size);
  if (g_verbose)
    dump_memory(buf->start + offset, actual_size, offset, 1, desc);
  buf->size += actual_size;
}

static void dump_output_buffer(OutputBuffer* buf) {
  dump_memory(buf->start, buf->size, 0, 1, NULL);
}

static void write_output_buffer(OutputBuffer* buf, const char* filename) {
  FILE* out = (strcmp(filename, "-") != 0) ? fopen(filename, "wb") : stdout;
  if (!out)
    FATAL("unable to open %s\n", filename);
  if (fwrite(buf->start, 1, buf->size, out) != buf->size)
    FATAL("unable to write %zd bytes\n", buf->size);
  fclose(out);
}

static void destroy_output_buffer(OutputBuffer* buf) {
  free(buf->start);
}

static int get_binding_by_name(BindingVector* bindings, const char* name) {
  int i;
  for (i = 0; i < bindings->size; ++i) {
    Binding* binding = &bindings->data[i];
    if (binding->name && strcmp(name, binding->name) == 0)
      return i;
  }
  return -1;
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

static int read_uint32_token(Token t, uint32_t* out) {
  const char* p = t.range.start.pos;
  return read_uint32(&p, t.range.end.pos, out);
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
                    if (!isxdigit(*t->loc.pos))
                      FATAL_AT(t->loc, "bad \\xx escape sequence\n");
                  } else {
                    FATAL_AT(t->loc, "eof in \\xx escape sequence\n");
                  }
                } else {
                  FATAL_AT(t->loc, "bad escape sequence\n");
                }
              } else {
                FATAL_AT(t->loc, "eof in escape sequence\n");
              }
              break;

            case '\n':
              FATAL_AT(t->loc, "newline in string\n");
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
    return 10 + (c - 'a');
  } else {
    assert(c >= 'A' && c <= 'F');
    return 10 + (c - 'A');
  }
}

static size_t string_contents_length(Token t) {
  assert(t.type == TOKEN_TYPE_STRING);
  const char* src = t.range.start.pos + 1;
  const char* end = t.range.end.pos - 1;
  size_t length = 0;
  while (src < end) {
    if (*src == '\\') {
      src++;
      switch (*src) {
        case 'n':
        case 't':
        case '\\':
        case '\'':
        case '\"':
          src++;
          length++;
          break;

        default:
          /* must be a hex sequence */
          src += 2;
          length++;
          break;
      }
    } else {
      src++;
      length++;
    }
  }
  return length;
}

static size_t copy_string_contents(Token t, char* dest, size_t size) {
  assert(t.type == TOKEN_TYPE_STRING);
  const char* src = t.range.start.pos + 1;
  const char* end = t.range.end.pos - 1;

  char* dest_start = dest;

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
  /* return the data length */
  return dest - dest_start;
}

static char* dup_string_contents(Token t) {
  assert(t.type == TOKEN_TYPE_STRING);
  const char* src = t.range.start.pos + 1;
  const char* end = t.range.end.pos - 1;
  /* Always allocate enough space for the entire string including the escape
   * characters. It will only get shorter, and this way we only have to iterate
   * through the string once. */
  size_t size = end - src + 1; /* +1 for \0 */
  char* result = malloc(size);
  size_t actual_size = copy_string_contents(t, result, size);
  result[actual_size] = '\0';
  return result;
}

static void expect(Token t, TokenType token_type, const char* desc) {
  if (t.type == TOKEN_TYPE_EOF) {
    FATAL_AT(t.range.start, "unexpected EOF\n");
  } else if (t.type != token_type) {
    FATAL_AT(t.range.start, "expected %s, not \"%.*s\"\n", desc,
             (int)(t.range.end.pos - t.range.start.pos), t.range.start.pos);
  }
}

static void unexpected_token(Token t) {
  if (t.type == TOKEN_TYPE_EOF) {
    FATAL_AT(t.range.start, " unexpected EOF\n");
  } else {
    FATAL_AT(t.range.start, "unexpected token \"%.*s\"\n",
             (int)(t.range.end.pos - t.range.start.pos), t.range.start.pos);
  }
}

static void expect_open(Token t) {
  expect(t, TOKEN_TYPE_OPEN_PAREN, "'('");
}

static void expect_close(Token t) {
  expect(t, TOKEN_TYPE_CLOSE_PAREN, "')'");
}

static void expect_atom(Token t) {
  expect(t, TOKEN_TYPE_ATOM, "ATOM");
}

static void expect_string(Token t) {
  expect(t, TOKEN_TYPE_STRING, "STRING");
}

static void expect_var_name(Token t) {
  expect_atom(t);
  if (t.range.end.pos - t.range.start.pos < 1 || t.range.start.pos[0] != '$')
    FATAL_AT(t.range.start, "expected name to begin with $\n");
}

static void check_opcode(SourceLocation loc, Opcode opcode) {
  if (opcode == OPCODE_INVALID)
    FATAL_AT(loc, "no opcode for instruction\n");
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
      if (alignment == 0 || (alignment & (alignment - 1)) != 0)
        FATAL_AT(t.range.start, "alignment must be power-of-two\n");

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

static void check_type(SourceLocation loc,
                       Type actual,
                       Type expected,
                       const char* desc) {
  if (actual != expected) {
    FATAL_AT(loc, "type mismatch%s. got %s, expected %s\n", desc,
             s_type_names[actual], s_type_names[expected]);
  }
}

static void check_type_arg(SourceLocation loc,
                           Type actual,
                           Type expected,
                           const char* desc,
                           int arg_index) {
  if (actual != expected) {
    FATAL_AT(loc, "type mismatch for argument %d of %s. got %s, expected %s\n",
             arg_index, desc, s_type_names[actual], s_type_names[expected]);
  }
}

static Type get_result_type(SourceLocation loc, Function* function) {
  switch (function->result_types.size) {
    case 0:
      return TYPE_VOID;

    case 1:
      return function->result_types.data[0];

    default:
      FATAL_AT(loc, "multiple return values currently unsupported\n");
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
      FATAL_AT(t.range.start, "unexpected eof\n");
    }
  }
}

static int parse_var(Tokenizer* tokenizer,
                     BindingVector* bindings,
                     int num_vars,
                     const char* desc) {
  Token t = read_token(tokenizer);
  expect_atom(t);

  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  if (end - p >= 1 && p[0] == '$') {
    /* var name */
    int i;
    for (i = 0; i < bindings->size; ++i) {
      Binding* binding = &bindings->data[i];
      const char* name = binding->name;
      if (name && strncmp(name, p, end - p) == 0)
        return binding->index;
    }
    FATAL_AT(t.range.start, "undefined %s variable \"%.*s\"\n", desc,
             (int)(end - p), p);
  } else {
    /* var index */
    uint32_t index;
    if (!read_uint32(&p, t.range.end.pos, &index) || p != t.range.end.pos) {
      FATAL_AT(t.range.start, "invalid var index\n");
    }

    if (index >= num_vars) {
      FATAL_AT(t.range.start, "%s variable out of range (max %d)\n", desc,
               num_vars);
    }

    return index;
  }
}

static int parse_function_var(Tokenizer* tokenizer, Module* module) {
  return parse_var(tokenizer, &module->function_bindings,
                   module->functions.size, "function");
}

static int parse_global_var(Tokenizer* tokenizer, Module* module) {
  return parse_var(tokenizer, &module->global_bindings, module->globals.size,
                   "global");
}

static int parse_local_var(Tokenizer* tokenizer, Function* function) {
  return parse_var(tokenizer, &function->local_bindings, function->locals.size,
                   "local");
}

/* Labels are indexed in reverse order (0 is the most recent, etc.), so handle
 * them separately */
static int parse_label_var(Tokenizer* tokenizer, Function* function) {
  Token t = read_token(tokenizer);
  expect_atom(t);

  const char* p = t.range.start.pos;
  const char* end = t.range.end.pos;
  if (end - p >= 1 && p[0] == '$') {
    /* var name */
    int i;
    for (i = 0; i < function->labels.size; ++i) {
      Binding* binding = &function->labels.data[i];
      const char* name = binding->name;
      if (name && strncmp(name, p, end - p) == 0)
        /* index is actually the current block depth */
        return binding->index;
    }
    FATAL_AT(t.range.start, "undefined label variable \"%.*s\"\n",
             (int)(end - p), p);
  } else {
    /* var index */
    uint32_t index;
    if (!read_uint32(&p, t.range.end.pos, &index) || p != t.range.end.pos) {
      FATAL_AT(t.range.start, "invalid var index\n");
    }

    if (index >= function->labels.size) {
      FATAL_AT(t.range.start, "label variable out of range (max %zd)\n",
               function->labels.size);
    }

    /* Index in reverse, so 0 is the most recent label. The stored "index" is
     * actually the block depth, so return that instead */
    index = (function->labels.size - 1) - index;
    return function->labels.data[index].index;
  }
}

static void parse_type(Tokenizer* tokenizer, Type* type) {
  Token t = read_token(tokenizer);
  if (!match_type(t, type))
    FATAL_AT(t.range.start, "expected type\n");
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
      if (!read_uint32(&p, end, &value))
        FATAL_AT(t.range.start, "invalid unsigned 32-bit int\n");
      out_u32(buf, value, "u32 literal");
      break;
    }

    case TYPE_I64: {
      uint64_t value;
      if (!read_uint64(&p, end, &value))
        FATAL_AT(t.range.start, "invalid unsigned 64-bit int\n");
      out_u64(buf, value, "u64 literal");
      break;
    }

    case TYPE_F32:
    case TYPE_F64: {
      double value;
      if (!read_double(&p, end, &value))
        FATAL_AT(t.range.start, "invalid double\n");
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
  expect_atom(t);

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
      check_type_arg(t.range.start, value0_type, op_info->type, "binary op", 0);
      Type value1_type = parse_expr(tokenizer, module, function, buf);
      check_type_arg(t.range.start, value1_type, op_info->type, "binary op", 1);
      assert(value0_type == value1_type);
      type = value0_type;
      expect_close(read_token(tokenizer));
      break;
    }

    case OP_BLOCK:
      out_opcode(buf, OPCODE_BLOCK);
      function->depth++;
      type = parse_block(tokenizer, module, function, buf);
      function->depth--;
      break;

    case OP_BREAK: {
      out_opcode(buf, OPCODE_BREAK);
      t = read_token(tokenizer);
      int depth = 0;
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        /* parse_label_var returns the depth counting up (so 0 is the topmost
         block of this function). We want the depth counting down (so 0 is the
         most recent block). */
        depth = (function->depth - 1) - parse_label_var(tokenizer, function);
        expect_close(read_token(tokenizer));
      } else if (function->labels.size == 0) {
        FATAL_AT(t.range.start, "label variable out of range (max 0)\n");
      }
      out_u8(buf, depth, "break depth");
      break;
    }

    case OP_CALL: {
      out_opcode(buf, OPCODE_CALL);
      int index = parse_function_var(tokenizer, module);
      out_leb128(buf, index, "func index");
      Function* callee = &module->functions.data[index];

      int num_args = 0;
      while (1) {
        Token t = read_token(tokenizer);
        if (t.type == TOKEN_TYPE_CLOSE_PAREN)
          break;
        rewind_token(tokenizer, t);
        if (++num_args > callee->num_args) {
          FATAL_AT(t.range.start,
                   "too many arguments to function. got %d, expected %d\n",
                   num_args, callee->num_args);
        }
        Type arg_type = parse_expr(tokenizer, module, function, buf);
        Type expected = callee->locals.data[num_args - 1].type;
        check_type_arg(t.range.start, arg_type, expected, "call", num_args - 1);
      }

      if (num_args < callee->num_args) {
        FATAL_AT(t.range.start,
                 "too few arguments to function. got %d, expected %d\n",
                 num_args, callee->num_args);
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
      out_opcode(buf, OPCODE_GET_LOCAL);
      int index = parse_local_var(tokenizer, function);
      Variable* variable = &function->locals.data[index];
      type = variable->type;
      out_leb128(buf, variable->index, "remapped local index");
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
      out_opcode(buf, OPCODE_BLOCK);

      t = read_token(tokenizer);
      Binding* binding = append_binding(&function->labels);
      binding->name = NULL;
      binding->index = function->depth++;

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
      pop_binding(&function->labels);
      function->depth--;
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
      type = module->globals.data[index].type;
      expect_close(read_token(tokenizer));
      break;
    }

    case OP_LOOP:
      out_opcode(buf, OPCODE_LOOP);
      function->depth++;
      type = parse_block(tokenizer, module, function, buf);
      function->depth--;
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
        if (++num_results > function->result_types.size) {
          FATAL_AT(t.range.start,
                   "too many return values. got %d, expected %zd\n",
                   num_results, function->result_types.size);
        }
        rewind_token(tokenizer, t);
        Type result_type = parse_expr(tokenizer, module, function, buf);
        Type expected = function->result_types.data[num_results - 1];
        check_type_arg(t.range.start, result_type, expected, "return",
                       num_results - 1);
      }

      if (num_results < function->result_types.size) {
        FATAL_AT(t.range.start, "too few return values. got %d, expected %zd\n",
                 num_results, function->result_types.size);
      }

      type = get_result_type(t.range.start, function);
      break;
    }

    case OP_SET_LOCAL: {
      out_opcode(buf, OPCODE_SET_LOCAL);
      int index = parse_local_var(tokenizer, function);
      Variable* variable = &function->locals.data[index];
      out_leb128(buf, variable->index, "remapped local index");
      type = parse_expr(tokenizer, module, function, buf);
      check_type(t.range.start, type, variable->type, "");
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
      Variable* variable = &module->globals.data[index];
      type = parse_expr(tokenizer, module, function, buf);
      check_type(t.range.start, type, variable->type, "");
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
  return type;
}

static int parse_func(Tokenizer* tokenizer,
                      Module* module,
                      Function* function,
                      OutputBuffer* buf) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    t = read_token(tokenizer);
  }

  int num_exprs = 0;
  Type type = TYPE_VOID;
  while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(t);
    Token open = t;
    t = read_token(tokenizer);
    expect_atom(t);

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
        ++num_exprs;
        break;
    }
    t = read_token(tokenizer);
  }

  Type result_type = get_result_type(t.range.start, function);
  if (result_type != TYPE_VOID)
    check_type(t.range.start, type, result_type, " in function result");

  return num_exprs;
}

static void preparse_binding_list(Tokenizer* tokenizer,
                                  VariableVector* variables,
                                  BindingVector* bindings,
                                  const char* desc) {
  Token t = read_token(tokenizer);
  Type type;
  if (match_type(t, &type)) {
    while (1) {
      Variable* variable = append_variable(variables);
      variable->type = type;
      variable->offset = 0;
      variable->index = 0;

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
    if (get_binding_by_name(bindings, name) != -1)
      FATAL_AT(t.range.start, "redefinition of %s \"%s\"\n", desc, name);

    Variable* variable = append_variable(variables);
    variable->type = type;
    variable->offset = 0;
    variable->index = 0;

    Binding* binding = append_binding(bindings);
    binding->name = name;
    binding->index = variables->size - 1;
  }
}

static void remap_locals(Function* function) {
  int max[NUM_TYPES] = {};
  int i;
  for (i = function->num_args; i < function->locals.size; ++i) {
    Variable* variable = &function->locals.data[i];
    max[variable->type]++;
  }

  /* Args don't need remapping */
  for (i = 0; i < function->num_args; ++i)
    function->locals.data[i].index = i;

  int start[NUM_TYPES];
  start[TYPE_I32] = function->num_args;
  start[TYPE_I64] = start[TYPE_I32] + max[TYPE_I32];
  start[TYPE_F32] = start[TYPE_I64] + max[TYPE_I64];
  start[TYPE_F64] = start[TYPE_F32] + max[TYPE_F32];

  int seen[NUM_TYPES] = {};
  for (i = function->num_args; i < function->locals.size; ++i) {
    Variable* variable = &function->locals.data[i];
    variable->index = start[variable->type] + seen[variable->type]++;
  }
}

static void preparse_func(Tokenizer* tokenizer, Module* module) {
  Function* function = append_function(&module->functions);
  memset(function, 0, sizeof(*function));

  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    char* name =
        strndup(t.range.start.pos, t.range.end.pos - t.range.start.pos);
    if (get_binding_by_name(&module->function_bindings, name) != -1)
      FATAL_AT(t.range.start, "redefinition of function \"%s\"\n", name);

    Binding* binding = append_binding(&module->function_bindings);
    binding->name = name;
    binding->index = module->functions.size - 1;
    t = read_token(tokenizer);
  }

  while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(t);
    t = read_token(tokenizer);
    expect_atom(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_PARAM:
        /* Don't allow adding params after locals */
        if (function->num_args != function->locals.size)
          FATAL_AT(t.range.start, "parameters must come before locals\n");
        preparse_binding_list(tokenizer, &function->locals,
                              &function->local_bindings, "function argument");
        function->num_args = function->locals.size;
        break;

      case OP_RESULT: {
        t = read_token(tokenizer);
        while (1) {
          Type type;
          if (!match_type(t, &type))
            unexpected_token(t);
          *(Type*)append_type(&function->result_types) = type;

          t = read_token(tokenizer);
          if (t.type == TOKEN_TYPE_CLOSE_PAREN)
            break;
        }
        break;
      }

      case OP_LOCAL:
        preparse_binding_list(tokenizer, &function->locals,
                              &function->local_bindings, "local");
        break;

      default:
        rewind_token(tokenizer, t);
        parse_generic(tokenizer);
        break;
    }
    t = read_token(tokenizer);
  }

  remap_locals(function);
}

static void preparse_module(Tokenizer* tokenizer, Module* module) {
  int seen_memory = 0;
  Token t = read_token(tokenizer);
  Token first = t;
  while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(t);
    t = read_token(tokenizer);
    expect_atom(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_FUNC:
        preparse_func(tokenizer, module);
        break;

      case OP_GLOBAL:
        preparse_binding_list(tokenizer, &module->globals,
                              &module->global_bindings, "global");
        break;

      case OP_MEMORY: {
        if (seen_memory)
          FATAL_AT(t.range.start, "only one memory block allowed\n");
        seen_memory = 1;
        t = read_token(tokenizer);
        expect_atom(t);
        if (!read_uint32_token(t, &module->initial_memory_size))
          FATAL_AT(t.range.start, "invalid initial memory size\n");

        t = read_token(tokenizer);
        if (t.type == TOKEN_TYPE_ATOM) {
          if (!read_uint32_token(t, &module->max_memory_size))
            FATAL_AT(t.range.start, "invalid max memory size\n");

          if (module->max_memory_size < module->initial_memory_size) {
            FATAL_AT(t.range.start,
                     "max size (%u) must be greater than or equal to initial "
                     "size (%u)\n",
                     module->max_memory_size, module->initial_memory_size);
          }

          t = read_token(tokenizer);
        } else {
          module->max_memory_size = module->initial_memory_size;
        }

        uint32_t last_segment_end = 0;
        while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
          expect_open(t);
          t = read_token(tokenizer);
          if (!match_atom(t, "segment"))
            unexpected_token(t);
          t = read_token(tokenizer);
          expect_atom(t);

          Segment* segment = append_segment(&module->segments);

          if (!read_uint32_token(t, &segment->address))
            FATAL_AT(t.range.start, "invalid memory segment address\n");

          if (segment->address < last_segment_end) {
            FATAL_AT(t.range.start,
                     "address (%u) less than end of previous segment (%u)\n",
                     segment->address, last_segment_end);
          }

          if (segment->address >= module->initial_memory_size) {
            FATAL_AT(t.range.start,
                     "address (%u) greater than initial memory size (%u)\n",
                     segment->address, module->initial_memory_size);
          }

          t = read_token(tokenizer);
          expect_string(t);
          segment->data = t;
          segment->size = string_contents_length(t);

          last_segment_end = segment->address + segment->size;

          if (last_segment_end > module->initial_memory_size) {
            FATAL_AT(
                t.range.start,
                "segment ends past the end of the initial memory size (%u)\n",
                module->initial_memory_size);
          }

          expect_close(read_token(tokenizer));
          t = read_token(tokenizer);
        }
        break;
      }

      default:
        parse_generic(tokenizer);
        break;
    }
    t = read_token(tokenizer);
  }
  rewind_token(tokenizer, first);
}

static void destroy_binding_list(BindingVector* bindings) {
  int i;
  for (i = 0; i < bindings->size; ++i)
    free(bindings->data[i].name);
  destroy_binding_vector(bindings);
}

static void destroy_module(Module* module) {
  int i;
  for (i = 0; i < module->functions.size; ++i) {
    Function* function = &module->functions.data[i];
    destroy_type_vector(&function->result_types);
    destroy_variable_vector(&function->locals);
    destroy_binding_list(&function->local_bindings);
    destroy_binding_list(&function->labels);
  }
  destroy_binding_list(&module->function_bindings);
  destroy_binding_list(&module->global_bindings);
  destroy_function_vector(&module->functions);
  destroy_variable_vector(&module->globals);
  for (i = 0; i < module->exports.size; ++i)
    free(module->exports.data[i].name);
  destroy_export_vector(&module->exports);
  destroy_segment_vector(&module->segments);
}

static void out_module_header(OutputBuffer* buf, Module* module) {
  out_u8(buf, log_two_u32(module->max_memory_size), "mem size log 2");
  out_u8(buf, DEFAULT_MEMORY_EXPORT, "export mem");
  out_u16(buf, module->globals.size, "num globals");
  out_u16(buf, module->functions.size, "num funcs");
  out_u16(buf, module->segments.size, "num data segments");

  int i;
  for (i = 0; i < module->globals.size; ++i) {
    Variable* global = &module->globals.data[i];
    if (g_verbose)
      printf("; global header %d\n", i);
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

  for (i = 0; i < module->functions.size; ++i) {
    Function* function = &module->functions.data[i];
    if (g_verbose)
      printf("; function header %d\n", i);

    out_u8(buf, function->num_args, "func num args");
    out_u8(buf,
           function->result_types.size ? function->result_types.data[0] : 0,
           "func result type");
    int j;
    for (j = 0; j < function->num_args; ++j)
      out_u8(buf, function->locals.data[j].type, "func arg type");
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
    for (j = function->num_args; j < function->locals.size; ++j)
      num_locals[function->locals.data[j].type]++;

    out_u16(buf, num_locals[TYPE_I32], "num local i32");
    out_u16(buf, num_locals[TYPE_I64], "num local i64");
    out_u16(buf, num_locals[TYPE_F32], "num local f32");
    out_u16(buf, num_locals[TYPE_F64], "num local f64");
    out_u8(buf, 0, "export func");
    out_u8(buf, 0, "func external");
  }

  for (i = 0; i < module->segments.size; ++i) {
    Segment* segment = &module->segments.data[i];
    if (g_verbose)
      printf("; segment header %d\n", i);
#define SEGMENT_DATA_OFFSET 4
    segment->offset = buf->size;
    out_u32(buf, segment->address, "segment address");
    out_u32(buf, 0, "segment data offset");
    out_u32(buf, segment->size, "segment size");
    out_u8(buf, 1, "segment init");
  }
}

static void out_module_footer(OutputBuffer* buf, Module* module) {
  int i;
  for (i = 0; i < module->segments.size; ++i) {
    if (g_verbose)
      printf("; segment data %d\n", i);
    Segment* segment = &module->segments.data[i];
    out_u32_at(buf, segment->offset + SEGMENT_DATA_OFFSET, buf->size,
               "FIXUP segment data offset");
    out_string_token(buf, segment->data, "segment data");
  }

  /* output name table */
  if (g_verbose)
    printf("; names\n");
  for (i = 0; i < module->exports.size; ++i) {
    Export* export = &module->exports.data[i];
    Function* function = &module->functions.data[export->index];
    out_u32_at(buf, function->offset, buf->size, "FIXUP func name offset");
    out_cstr(buf, export->name, "export name");
  }
}

void parse_module(Tokenizer* tokenizer) {
  Module module = {};
  preparse_module(tokenizer, &module);

  OutputBuffer output = {};
  init_output_buffer(&output, INITIAL_OUTPUT_BUFFER_CAPACITY);
  out_module_header(&output, &module);

  int function_index = 0;
  Token t = read_token(tokenizer);
  while (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    expect_open(t);
    t = read_token(tokenizer);
    expect_atom(t);

    const OpInfo* op_info = get_op_info(t);
    switch (op_info ? op_info->op_type : OP_NONE) {
      case OP_FUNC: {
        Function* function = &module.functions.data[function_index++];
        if (g_verbose)
          printf("; function data %d\n", function_index - 1);
        out_u32_at(&output, function->offset + CODE_START_OFFSET, output.size,
                   "FIXUP func code start offset");
        /* The v8-native-prototype requires all functions to have a toplevel
         block */
        out_opcode(&output, OPCODE_BLOCK);
        size_t offset = output.size;
        out_u8(&output, 0, "toplevel block num expressions");
        int num_exprs = parse_func(tokenizer, &module, function, &output);
        out_u8_at(&output, offset, num_exprs,
                  "FIXUP toplevel block num expressions");
        out_u32_at(&output, function->offset + CODE_END_OFFSET, output.size,
                   "FIXUP func code end offset");
        break;
      }

      case OP_EXPORT: {
        t = read_token(tokenizer);
        expect_string(t);
        int index = parse_function_var(tokenizer, &module);

        Export* export = append_export(&module.exports);
        export->name = dup_string_contents(t);
        export->index = index;

        Function* exported = &module.functions.data[index];
        out_u8_at(&output, exported->offset + FUNCTION_EXPORTED_OFFSET, 1,
                  "FIXUP func exported");
        expect_close(read_token(tokenizer));
        break;
      }

      case OP_TABLE:
        parse_generic(tokenizer);
        break;

      case OP_GLOBAL:
      case OP_MEMORY:
        /* Skip, this has already been pre-parsed */
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

  if (g_outfile) {
    /* TODO(binji): better way to prevent multiple output */
    static int s_already_output = 0;
    if (s_already_output)
      FATAL("Can't write multiple modules to output\n");
    s_already_output = 1;
    write_output_buffer(&output, g_outfile);
  }

  destroy_output_buffer(&output);
  destroy_module(&module);
}

void parse_file(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  while (t.type != TOKEN_TYPE_EOF) {
    expect_open(t);
    t = read_token(tokenizer);
    expect_atom(t);

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

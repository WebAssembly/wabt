#include <assert.h>
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

typedef struct NameTypePair {
  const char* name;
  Type type;
} NameTypePair;

typedef struct NameType2Pair {
  const char* name;
  Type in_type;
  Type out_type;
} NameType2Pair;

typedef struct NameMemTypePair {
  const char* name;
  MemType type;
} NameMemTypePair;

typedef struct NameMemType2Pair {
  const char* name;
  MemType mem_type;
  Type type;
} NameMemType2Pair;

static int g_verbose;
static const char* g_filename;

static NameTypePair s_unary_ops[] = {
    {"f32.neg", TYPE_F32},   {"f64.neg", TYPE_F64},   {"f32.abs", TYPE_F32},
    {"f64.abs", TYPE_F64},   {"f32.sqrt", TYPE_F32},  {"f64.sqrt", TYPE_F64},
    {"i32.not", TYPE_I32},   {"i64.not", TYPE_I64},   {"f32.not", TYPE_F32},
    {"f64.not", TYPE_F64},   {"i32.clz", TYPE_I32},   {"i64.clz", TYPE_I64},
    {"i32.ctz", TYPE_I32},   {"i64.ctz", TYPE_I64},   {"f32.ceil", TYPE_F32},
    {"f64.ceil", TYPE_F64},  {"f32.floor", TYPE_F32}, {"f64.floor", TYPE_F64},
    {"f32.trunc", TYPE_F32}, {"f64.trunc", TYPE_F64}, {"f32.nearest", TYPE_F32},
    {"f64.nearest", TYPE_F64},
};

static NameTypePair s_binary_ops[] = {
    {"i32.add", TYPE_I32},      {"i64.add", TYPE_I64},
    {"f32.add", TYPE_F32},      {"f64.add", TYPE_F64},
    {"i32.sub", TYPE_I32},      {"i64.sub", TYPE_I64},
    {"f32.sub", TYPE_F32},      {"f64.sub", TYPE_F64},
    {"i32.mul", TYPE_I32},      {"i64.mul", TYPE_I64},
    {"f32.mul", TYPE_F32},      {"f64.mul", TYPE_F64},
    {"i32.div_s", TYPE_I32},    {"i64.div_s", TYPE_I64},
    {"i32.div_u", TYPE_I32},    {"i64.div_u", TYPE_I64},
    {"f32.div", TYPE_F32},      {"f64.div", TYPE_F64},
    {"i32.rem_s", TYPE_I32},    {"i64.rem_s", TYPE_I64},
    {"i32.rem_u", TYPE_I32},    {"i64.rem_u", TYPE_I64},
    {"i32.and", TYPE_I32},      {"i64.and", TYPE_I64},
    {"i32.or", TYPE_I32},       {"i64.or", TYPE_I64},
    {"i32.xor", TYPE_I32},      {"i64.xor", TYPE_I64},
    {"i32.shl", TYPE_I32},      {"i64.shl", TYPE_I64},
    {"i32.shr", TYPE_I32},      {"i64.shr", TYPE_I64},
    {"i32.sar", TYPE_I32},      {"i64.sar", TYPE_I64},
    {"f32.copysign", TYPE_F32}, {"f64.copysign", TYPE_F64},
};

static NameTypePair s_compare_ops[] = {
    {"i32.eq", TYPE_I32},   {"i64.eq", TYPE_I64},   {"f32.eq", TYPE_F32},
    {"f64.eq", TYPE_F64},   {"i32.neq", TYPE_I32},  {"i64.neq", TYPE_I64},
    {"f32.neq", TYPE_F32},  {"f64.neq", TYPE_F64},  {"i32.lt_s", TYPE_I32},
    {"i64.lt_s", TYPE_I64}, {"i32.lt_u", TYPE_I32}, {"i64.lt_u", TYPE_I64},
    {"f32.lt", TYPE_F32},   {"f64.lt", TYPE_F64},   {"i32.le_s", TYPE_I32},
    {"i64.le_s", TYPE_I64}, {"i32.le_u", TYPE_I32}, {"i64.le_u", TYPE_I64},
    {"f32.le", TYPE_F32},   {"f64.le", TYPE_F64},   {"i32.gt_s", TYPE_I32},
    {"i64.gt_s", TYPE_I64}, {"i32.gt_u", TYPE_I32}, {"i64.gt_u", TYPE_I64},
    {"f32.gt", TYPE_F32},   {"f64.gt", TYPE_F64},   {"i32.ge_s", TYPE_I32},
    {"i64.ge_s", TYPE_I64}, {"i32.ge_u", TYPE_I32}, {"i64.ge_u", TYPE_I64},
    {"f32.ge", TYPE_F32},   {"f64.ge", TYPE_F64},
};

static NameType2Pair s_convert_ops[] = {
    {"i64.extend_s/i32", TYPE_I32, TYPE_I64},
    {"i64.extend_u/i32", TYPE_I32, TYPE_I64},
    {"i32.wrap/i64", TYPE_I64, TYPE_I32},
    {"f32.convert_s/i32", TYPE_I32, TYPE_F32},
    {"f32.convert_u/i32", TYPE_I32, TYPE_F32},
    {"f64.convert_s/i32", TYPE_I32, TYPE_F64},
    {"f64.convert_u/i32", TYPE_I32, TYPE_F64},
    {"f32.convert_s/i64", TYPE_I64, TYPE_F32},
    {"f32.convert_u/i64", TYPE_I64, TYPE_F32},
    {"f64.convert_s/i64", TYPE_I64, TYPE_F64},
    {"f64.convert_u/i64", TYPE_I64, TYPE_F64},
    {"i32.trunc_s/f32", TYPE_F32, TYPE_I32},
    {"i32.trunc_u/f32", TYPE_F32, TYPE_I32},
    {"i64.trunc_s/f32", TYPE_F32, TYPE_I64},
    {"i64.trunc_u/f32", TYPE_F32, TYPE_I64},
    {"i32.trunc_s/f64", TYPE_F64, TYPE_I32},
    {"i32.trunc_u/f64", TYPE_F64, TYPE_I32},
    {"i64.trunc_s/f64", TYPE_F64, TYPE_I64},
    {"i64.trunc_u/f64", TYPE_F64, TYPE_I64},
    {"f64.promote/f32", TYPE_F32, TYPE_F64},
    {"f32.demote/f64", TYPE_F64, TYPE_F32},
};

static NameType2Pair s_cast_ops[] = {
    {"f32.reinterpret/i32", TYPE_I32, TYPE_F32},
    {"i32.reinterpret/f32", TYPE_F32, TYPE_I32},
    {"f64.reinterpret/i64", TYPE_I64, TYPE_F64},
    {"i64.reinterpret/f64", TYPE_F64, TYPE_I64},
};

static NameTypePair s_const_ops[] = {
    {"i32.const", TYPE_I32},
    {"i64.const", TYPE_I64},
    {"f32.const", TYPE_F32},
    {"f64.const", TYPE_F64},
};

static NameTypePair s_switch_ops[] = {
    {"i32.switch", TYPE_I32},
    {"i64.switch", TYPE_I64},
    {"f32.switch", TYPE_F32},
    {"f64.switch", TYPE_F64},
};

static NameMemType2Pair s_load_ops[] = {
    {"i32.load_s/i8", MEM_TYPE_I8, TYPE_I32},
    {"i32.load_u/i8", MEM_TYPE_I8, TYPE_I32},
    {"i32.load_s/i16", MEM_TYPE_I16, TYPE_I32},
    {"i32.load_u/i16", MEM_TYPE_I16, TYPE_I32},
    {"i32.load_s/i32", MEM_TYPE_I32, TYPE_I32},
    {"i32.load_u/i32", MEM_TYPE_I32, TYPE_I32},
    {"i64.load_s/i8", MEM_TYPE_I8, TYPE_I64},
    {"i64.load_u/i8", MEM_TYPE_I8, TYPE_I64},
    {"i64.load_s/i16", MEM_TYPE_I16, TYPE_I64},
    {"i64.load_u/i16", MEM_TYPE_I16, TYPE_I64},
    {"i64.load_s/i32", MEM_TYPE_I32, TYPE_I64},
    {"i64.load_u/i32", MEM_TYPE_I32, TYPE_I64},
    {"i64.load_s/i64", MEM_TYPE_I64, TYPE_I64},
    {"i64.load_u/i64", MEM_TYPE_I64, TYPE_I64},
    {"i32.load/i8", MEM_TYPE_I8, TYPE_I32},
    {"i32.load/i16", MEM_TYPE_I16, TYPE_I32},
    {"i32.load/i32", MEM_TYPE_I32, TYPE_I32},
    {"i64.load/i8", MEM_TYPE_I8, TYPE_I64},
    {"i64.load/i16", MEM_TYPE_I16, TYPE_I64},
    {"i64.load/i32", MEM_TYPE_I32, TYPE_I64},
    {"i64.load/i64", MEM_TYPE_I64, TYPE_I64},
    {"f32.load/f32", MEM_TYPE_F32, TYPE_F32},
    {"f64.load/f32", MEM_TYPE_F32, TYPE_F64},
    {"f64.load/f64", MEM_TYPE_F64, TYPE_F64},
};

static NameMemType2Pair s_store_ops[] = {
    {"i32.store_s/i8", MEM_TYPE_I8, TYPE_I32},
    {"i32.store_u/i8", MEM_TYPE_I8, TYPE_I32},
    {"i32.store_s/i16", MEM_TYPE_I16, TYPE_I32},
    {"i32.store_u/i16", MEM_TYPE_I16, TYPE_I32},
    {"i32.store_s/i32", MEM_TYPE_I32, TYPE_I32},
    {"i32.store_u/i32", MEM_TYPE_I32, TYPE_I32},
    {"i64.store_s/i8", MEM_TYPE_I8, TYPE_I64},
    {"i64.store_u/i8", MEM_TYPE_I8, TYPE_I64},
    {"i64.store_s/i16", MEM_TYPE_I16, TYPE_I64},
    {"i64.store_u/i16", MEM_TYPE_I16, TYPE_I64},
    {"i64.store_s/i32", MEM_TYPE_I32, TYPE_I64},
    {"i64.store_u/i32", MEM_TYPE_I32, TYPE_I64},
    {"i64.store_s/i64", MEM_TYPE_I64, TYPE_I64},
    {"i64.store_u/i64", MEM_TYPE_I64, TYPE_I64},
    {"i32.store/i8", MEM_TYPE_I8, TYPE_I32},
    {"i32.store/i16", MEM_TYPE_I16, TYPE_I32},
    {"i32.store/i32", MEM_TYPE_I32, TYPE_I32},
    {"i64.store/i8", MEM_TYPE_I8, TYPE_I64},
    {"i64.store/i16", MEM_TYPE_I16, TYPE_I64},
    {"i64.store/i32", MEM_TYPE_I32, TYPE_I64},
    {"i64.store/i64", MEM_TYPE_I64, TYPE_I64},
    {"f32.store/f32", MEM_TYPE_F32, TYPE_F32},
    {"f64.store/f32", MEM_TYPE_F32, TYPE_F64},
    {"f64.store/f64", MEM_TYPE_F64, TYPE_F64},
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
              }
              break;

            case '\n':
              FATAL("newline in string\n");
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

#if 0
static void print_tokens(Tokenizer* tokenizer) {
  while (1) {
    Token token = read_token(&tokenizer);
    fprintf(stderr, "[%d:%d]:[%d:%d]: ", token.range.start.line,
            token.range.start.col, token.range.end.line, token.range.end.col);
    switch (token.type) {
      case TOKEN_TYPE_EOF:
        fprintf(stderr, "EOF\n");
        return;

      case TOKEN_TYPE_OPEN_PAREN:
        fprintf(stderr, "OPEN_PAREN\n");
        break;

      case TOKEN_TYPE_CLOSE_PAREN:
        fprintf(stderr, "CLOSE_PAREN\n");
        break;

      case TOKEN_TYPE_ATOM:
        fprintf(stderr, "ATOM: %.*s\n",
                (int)(token.range.end.pos - token.range.start.pos),
                token.range.start.pos);
        break;

      case TOKEN_TYPE_STRING:
        fprintf(stderr, "STRING: %.*s\n",
                (int)(token.range.end.pos - token.range.start.pos),
                token.range.start.pos);
        break;
    }
  }
}
#endif

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

  if (t.range.end.pos - t.range.start.pos < 1 ||
      t.range.start.pos[0] != '$') {
    FATAL("%d:%d: expected name to begin with $\n", t.range.start.line,
          t.range.start.col);
  }
}

static int match_atom(Token t, const char* s) {
  return strncmp(t.range.start.pos, s, t.range.end.pos - t.range.start.pos) ==
         0;
}

static int match_atom_prefix(Token t, const char* s, size_t slen) {
  size_t len = t.range.end.pos - t.range.start.pos;
  if (len > slen)
    len = slen;
  return strncmp(t.range.start.pos, s, slen) == 0;
}

static int match_unary(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_unary_ops); ++i) {
    if (match_atom(t, s_unary_ops[i].name)) {
      *type = s_unary_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_binary(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_binary_ops); ++i) {
    if (match_atom(t, s_binary_ops[i].name)) {
      *type = s_binary_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_compare(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_compare_ops); ++i) {
    if (match_atom(t, s_compare_ops[i].name)) {
      *type = s_compare_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_convert(Token t, Type* in_type, Type* out_type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_convert_ops); ++i) {
    if (match_atom(t, s_convert_ops[i].name)) {
      *in_type = s_convert_ops[i].in_type;
      *out_type = s_convert_ops[i].out_type;
      return 1;
    }
  }
  return 0;
}

static int match_cast(Token t, Type* in_type, Type* out_type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_cast_ops); ++i) {
    if (match_atom(t, s_cast_ops[i].name)) {
      *in_type = s_cast_ops[i].in_type;
      *out_type = s_cast_ops[i].out_type;
      return 1;
    }
  }
  return 0;
}

static int match_const(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_const_ops); ++i) {
    if (match_atom(t, s_const_ops[i].name)) {
      *type = s_const_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_switch(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_switch_ops); ++i) {
    if (match_atom(t, s_switch_ops[i].name)) {
      *type = s_switch_ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_type(Token t, Type* type) {
  int i;
  for (i = 0; i < ARRAY_SIZE(s_types); ++i) {
    if (match_atom(t, s_types[i].name)) {
      if (type) {
        *type = s_types[i].type;
      }
      return 1;
    }
  }
  return 0;
}

static int match_load_store(Token t,
                            MemType* mem_type,
                            Type* type,
                            NameMemType2Pair* ops,
                            size_t num_ops) {
  int i;
  for (i = 0; i < num_ops; ++i) {
    size_t len = t.range.end.pos - t.range.start.pos;
    size_t name_len = strlen(ops[i].name);
    if (match_atom_prefix(t, ops[i].name, name_len)) {
      if (len >= name_len + 1 && t.range.start.pos[name_len] == '/') {
        /* might have an alignment */
        const char* p = &t.range.start.pos[name_len + 1];
        uint32_t alignment;
        if (!read_uint32(&p, t.range.end.pos, &alignment))
          return 0;
        /* check that alignment is power-of-two */
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
          FATAL("%d:%d: alignment must be power-of-two\n", t.range.start.line,
                t.range.start.col);
        }
      } else if (len != name_len) {
        /* bad suffix */
        return 0;
      }

      *mem_type = ops[i].mem_type;
      *type = ops[i].type;
      return 1;
    }
  }
  return 0;
}

static int match_load(Token t, MemType* mem_type, Type* type) {
  return match_load_store(t, mem_type, type, s_load_ops,
                          ARRAY_SIZE(s_load_ops));
}

static int match_store(Token t, MemType* mem_type, Type* type) {
  return match_load_store(t, mem_type, type, s_store_ops,
                          ARRAY_SIZE(s_store_ops));
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

static int parse_arg_var(Tokenizer* tokenizer, Function* function) {
  return parse_var(tokenizer, function->locals, function->num_args,
                   "function argument");
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
                       Function* function);

static Type parse_block(Tokenizer* tokenizer,
                        Module* module,
                        Function* function) {
  Type type;
  while (1) {
    type = parse_expr(tokenizer, module, function);
    Token t = read_token(tokenizer);
    if (t.type == TOKEN_TYPE_CLOSE_PAREN)
      break;
    rewind_token(tokenizer, t);
  }
  return type;
}

static void parse_literal(Tokenizer* tokenizer, Type type) {
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
      break;
    }

    case TYPE_I64: {
      uint64_t value;
      if (!read_uint64(&p, end, &value)) {
        FATAL("%d:%d: invalid unsigned 64-bit int\n", t.range.start.line,
              t.range.start.col);
      }
      break;
    }

    case TYPE_F32:
    case TYPE_F64: {
      double value;
      if (!read_double(&p, end, &value)) {
        FATAL("%d:%d: invalid double\n", t.range.start.line, t.range.start.col);
      }
      break;
    }

    default:
      assert(0);
  }
}

static Type parse_switch(Tokenizer* tokenizer,
                         Module* module,
                         Function* function,
                         Type in_type) {
  int num_cases = 0;
  Type cond_type = parse_expr(tokenizer, module, function);
  check_type(tokenizer->loc, cond_type, in_type, " in switch condition");
  Type type = TYPE_VOID;
  Token t = read_token(tokenizer);
  while (1) {
    expect_open(t);
    Token open = t;
    t = read_token(tokenizer);
    expect_atom(t);
    if (match_atom(t, "case")) {
      parse_literal(tokenizer, in_type);
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        Type value_type;
        while (1) {
          value_type = parse_expr(tokenizer, module, function);
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
      Type value_type = parse_expr(tokenizer, module, function);
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
                       Function* function) {
  Type type = TYPE_VOID;
  expect_open(read_token(tokenizer));
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    Type in_type;
    MemType mem_type;
    if (match_atom(t, "nop")) {
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "block")) {
      type = parse_block(tokenizer, module, function);
    } else if (match_atom(t, "if")) {
      Type cond_type = parse_expr(tokenizer, module, function);
      check_type(tokenizer->loc, cond_type, TYPE_I32, " of condition");
      Type true_type = parse_expr(tokenizer, module, function);
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        Type false_type = parse_expr(tokenizer, module, function);
        if (true_type == TYPE_VOID || false_type == TYPE_VOID) {
          type = TYPE_VOID;
        } else {
          check_type(tokenizer->loc, false_type, true_type,
                     " between true and false branches");
        }
        expect_close(read_token(tokenizer));
      } else {
        type = true_type;
      }
    } else if (match_atom(t, "loop")) {
      type = parse_block(tokenizer, module, function);
    } else if (match_atom(t, "label")) {
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
      type = parse_block(tokenizer, module, function);
    } else if (match_atom(t, "break")) {
      t = read_token(tokenizer);
      if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
        rewind_token(tokenizer, t);
        /* TODO(binji): how to check that the given label is a parent? */
        parse_label_var(tokenizer, function);
        expect_close(read_token(tokenizer));
      }
    } else if (match_switch(t, &in_type)) {
      type = parse_switch(tokenizer, module, function, in_type);
    } else if (match_atom(t, "call")) {
      int index = parse_function_var(tokenizer, module);
      Function* callee = &module->functions[index];

      int num_args = 0;
      while (1) {
        Token t = read_token(tokenizer);
        if (t.type == TOKEN_TYPE_CLOSE_PAREN)
          break;
        rewind_token(tokenizer, t);
        if (++num_args > callee->num_args) {
          FATAL("%d:%d: too many arguments to function. got %d, expected %d\n",
                t.range.start.line, t.range.start.col, num_args,
                callee->num_args);
        }
        Type arg_type = parse_expr(tokenizer, module, function);
        Type expected = callee->locals[num_args - 1].type;
        check_type_arg(t.range.start, arg_type, expected, "call", num_args - 1);
      }

      if (num_args < callee->num_args) {
        FATAL("%d:%d: too few arguments to function. got %d, expected %d\n",
              t.range.start.line, t.range.start.col, num_args,
              callee->num_args);
      }

      type = get_result_type(t.range.start, callee);
    } else if (match_atom(t, "call_indirect")) {
      /* TODO(binji) */
    } else if (match_atom(t, "return")) {
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
        Type result_type = parse_expr(tokenizer, module, function);
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
    } else if (match_atom(t, "destruct")) {
      /* TODO(binji) */
    } else if (match_atom(t, "getparam")) {
      int index = parse_arg_var(tokenizer, function);
      type = function->locals[index].type;
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "get_local")) {
      int index = parse_local_var(tokenizer, function);
      type = function->locals[index].type;
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "set_local")) {
      int index = parse_local_var(tokenizer, function);
      Binding* binding = &function->locals[index];
      type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, type, binding->type, "");
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "load_global")) {
      int index = parse_global_var(tokenizer, module);
      type = module->globals[index].type;
      expect_close(read_token(tokenizer));
    } else if (match_atom(t, "store_global")) {
      int index = parse_global_var(tokenizer, module);
      Binding* binding = &module->globals[index];
      type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, type, binding->type, "");
      expect_close(read_token(tokenizer));
    } else if (match_load(t, &mem_type, &type)) {
      Type index_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, index_type, TYPE_I32, " of load index");
      expect_close(read_token(tokenizer));
    } else if (match_store(t, &mem_type, &type)) {
      Type index_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, index_type, TYPE_I32, " of store index");
      Type value_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, value_type, type, " of store value");
      expect_close(read_token(tokenizer));
    } else if (match_const(t, &type)) {
      parse_literal(tokenizer, type);
      expect_close(read_token(tokenizer));
    } else if (match_unary(t, &type)) {
      Type value_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, value_type, type, " of unary op");
      type = value_type;
      expect_close(read_token(tokenizer));
    } else if (match_binary(t, &type)) {
      Type value0_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value0_type, type, "binary op", 0);
      Type value1_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value1_type, type, "binary op", 1);
      assert(value0_type == value1_type);
      type = value0_type;
      expect_close(read_token(tokenizer));
    } else if (match_compare(t, &in_type)) {
      Type value0_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value0_type, in_type, "compare op", 0);
      Type value1_type = parse_expr(tokenizer, module, function);
      check_type_arg(t.range.start, value1_type, in_type, "compare op", 1);
      type = TYPE_I32;
      expect_close(read_token(tokenizer));
    } else if (match_convert(t, &in_type, &type)) {
      Type value_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, value_type, in_type, " of convert op");
      expect_close(read_token(tokenizer));
    } else if (match_cast(t, &in_type, &type)) {
      Type value_type = parse_expr(tokenizer, module, function);
      check_type(t.range.start, value_type, in_type, " of cast op");
      expect_close(read_token(tokenizer));
    } else {
      unexpected_token(t);
    }
  }
  return type;
}

static void parse_func(Tokenizer* tokenizer,
                       Module* module,
                       Function* function) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    t = read_token(tokenizer);
  }

  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      Token open = t;
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "param") || match_atom(t, "result") ||
            match_atom(t, "local")) {
          /* Skip, this has already been pre-parsed */
          parse_generic(tokenizer);
        } else {
          rewind_token(tokenizer, open);
          Type type = parse_expr(tokenizer, module, function);
          Type result_type = get_result_type(t.range.start, function);
          if (result_type != TYPE_VOID) {
            check_type(t.range.start, type, result_type, " in function result");
          }
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
  }
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

  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      Type type;
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "param")) {
          /* Don't allow adding params after locals */
          if (function->num_args != function->num_locals) {
            FATAL("%d:%d: parameters must come before locals\n",
                  t.range.start.line, t.range.start.col);
          }
          preparse_binding_list(tokenizer, &function->locals,
                                &function->num_args, "function argument");
          function->num_locals = function->num_args;
        } else if (match_atom(t, "result")) {
          t = read_token(tokenizer);
          if (match_type(t, &type)) {
            while (1) {
              realloc_list((void**)&function->result_types,
                           &function->num_results, sizeof(Type));
              function->result_types[function->num_results - 1] = type;

              t = read_token(tokenizer);
              if (t.type == TOKEN_TYPE_CLOSE_PAREN)
                break;
              else if (!match_type(t, &type))
                unexpected_token(t);
            }
          } else {
            unexpected_token(t);
          }
        } else if (match_atom(t, "local")) {
          preparse_binding_list(tokenizer, &function->locals,
                                &function->num_locals, "local");
        } else {
          rewind_token(tokenizer, t);
          parse_generic(tokenizer);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
  }
}

static void preparse_module(Tokenizer* tokenizer, Module* module) {
  Token t = read_token(tokenizer);
  Token first = t;
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "func")) {
          preparse_func(tokenizer, module);
        } else if (match_atom(t, "global")) {
          preparse_binding_list(tokenizer, &module->globals,
                                &module->num_globals, "global");
        } else {
          parse_generic(tokenizer);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
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
  free(module->exports);
}

static void parse_module(Tokenizer* tokenizer) {
  Module module = {};
  preparse_module(tokenizer, &module);

  int function_index = 0;
  Token t = read_token(tokenizer);
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "func")) {
          Function* function = &module.functions[function_index++];
          parse_func(tokenizer, &module, function);
        } else if (match_atom(t, "global")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "export")) {
          expect_string(read_token(tokenizer));
          parse_function_var(tokenizer, &module);
          expect_close(read_token(tokenizer));
        } else if (match_atom(t, "table")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "memory")) {
          parse_generic(tokenizer);
        } else {
          unexpected_token(t);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_CLOSE_PAREN) {
      break;
    } else {
      unexpected_token(t);
    }
  }
  destroy_module(&module);
}

static void parse(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "module")) {
          parse_module(tokenizer);
        } else if (match_atom(t, "asserteq")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "invoke")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "assertinvalid")) {
          parse_generic(tokenizer);
        } else {
          unexpected_token(t);
        }
      } else {
        unexpected_token(t);
      }
      t = read_token(tokenizer);
    } else if (t.type == TOKEN_TYPE_EOF) {
      break;
    } else {
      unexpected_token(t);
    }
  }
}

enum {
  FLAG_VERBOSE,
  FLAG_HELP,
  NUM_FLAGS
};

static struct option g_long_options[] = {
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
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
    c = getopt_long(argc, argv, "vh", g_long_options, &option_index);
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
            /* Handled above by goto */
            assert(0);
        }
        break;

      case 'v':
        g_verbose++;
        break;

      case 'h':
        usage(argv[0]);

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

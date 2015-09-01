#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABS_TO_SPACES 8

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
              fprintf(stderr, "newline in string\n");
              exit(1);
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

#if 0
static void expect_open(Token t) {
  if (t.type != TOKEN_TYPE_OPEN_PAREN) {
    fprintf(stderr, "%d:%d: expected '(', not \"%.*s\"\n", t.range.start.line,
            t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
            t.range.start.pos);
    exit(1);
  }
}
#endif

static void expect_close(Token t) {
  if (t.type != TOKEN_TYPE_CLOSE_PAREN) {
    fprintf(stderr, "%d:%d: expected ')', not \"%.*s\"\n", t.range.start.line,
            t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
            t.range.start.pos);
    exit(1);
  }
}


static void expect_atom(Token t) {
  if (t.type != TOKEN_TYPE_ATOM) {
    fprintf(stderr, "%d:%d: expected ATOM, not \"%.*s\"\n", t.range.start.line,
            t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
            t.range.start.pos);
    exit(1);
  }
}

static int match_atom(Token t, const char* s) {
  return strncmp(t.range.start.pos, s, t.range.end.pos - t.range.start.pos) ==
         0;
}

static void unexpected_token(Token t) {
  fprintf(stderr, "%d:%d: unexpected token \"%.*s\"\n", t.range.start.line,
          t.range.start.col, (int)(t.range.end.pos - t.range.start.pos),
          t.range.start.pos);
  exit(1);
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
      fprintf(stderr, "unexpected eof.\n");
      exit(1);
    }
  }
}

static void parse_expr(Tokenizer* tokenizer) {
  parse_generic(tokenizer);
}

static void parse_param(Tokenizer* tokenizer) {
  expect_atom(read_token(tokenizer));
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_CLOSE_PAREN)
    return;
  expect_atom(t);
  expect_close(read_token(tokenizer));
}

static void parse_result(Tokenizer* tokenizer) {
  expect_atom(read_token(tokenizer));
  expect_close(read_token(tokenizer));
}

static void parse_local(Tokenizer* tokenizer) {
  expect_atom(read_token(tokenizer));
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_CLOSE_PAREN)
    return;
  expect_atom(t);
  expect_close(read_token(tokenizer));
}

static void parse_func(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  if (t.type == TOKEN_TYPE_ATOM) {
    /* named function */
    t = read_token(tokenizer);
  }

  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "param")) {
          parse_param(tokenizer);
        } else if (match_atom(t, "result")) {
          parse_result(tokenizer);
        } else if (match_atom(t, "local")) {
          parse_local(tokenizer);
        } else {
          rewind_token(tokenizer, t);
          parse_expr(tokenizer);
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

static void parse_module(Tokenizer* tokenizer) {
  Token t = read_token(tokenizer);
  while (1) {
    if (t.type == TOKEN_TYPE_OPEN_PAREN) {
      t = read_token(tokenizer);
      if (t.type == TOKEN_TYPE_ATOM) {
        if (match_atom(t, "func")) {
          parse_func(tokenizer);
        } else if (match_atom(t, "global")) {
          parse_generic(tokenizer);
        } else if (match_atom(t, "export")) {
          parse_generic(tokenizer);
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

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s [file.wasm]\n", argv[0]);
    return 0;
  }

  const char* filename = argv[1];
  FILE* f = fopen(filename, "rb");
  if (!f) {
    fprintf(stderr, "unable to read %s\n", filename);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  void* data = malloc(fsize);
  if (!data) {
    fprintf(stderr, "unable to alloc %zd bytes\n", fsize);
    return 1;
  }
  if (fread(data, 1, fsize, f) != fsize) {
    fprintf(stderr, "unable to read %zd bytes from %s\n", fsize, filename);
    return 1;
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

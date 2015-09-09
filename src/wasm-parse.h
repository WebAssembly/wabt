#ifndef WASM_PARSE_H
#define WASM_PARSE_H
//#include "wasm.h"

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

extern int g_verbose;
extern const char* g_outfile;
extern int g_dump_module;

void parse_module(Tokenizer* tokenizer);
void parse_file(Tokenizer* tokenizer);

#endif /* WASM_PARSE_H */

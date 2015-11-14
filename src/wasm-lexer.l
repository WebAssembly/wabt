%{
#include "wasm-tokens.h"

#define YYSTYPE WasmToken
#define YYLTYPE WasmLocation

typedef struct WasmToken {
  const char* start;
  const char* end;
} WasmToken;

typedef struct WasmLocation {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} WasmLocation;

typedef struct WasmScannerExtra {
  int column;
  int comment_nesting;
} WasmScannerExtra;

#define EXTRA(scanner) ((WasmScannerExtra*)yyget_extra(scanner))
#define COMMENT_NESTING(scanner) (EXTRA(scanner)->comment_nesting)
#define COLUMN(scanner) (EXTRA(scanner)->column)
#define YY_USER_ACTION                                 \
  {                                                    \
    WasmScannerExtra* extra = EXTRA(yyscanner);        \
    yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = extra->column;              \
    extra->column += yyleng;                           \
    yylloc->last_column = extra->column;               \
    yylval->start = yytext;                            \
    yylval->end = yytext + yyleng;                     \
  }

%}

%option bison-bridge
%option bison-locations
%option yylineno
%option reentrant

%x LINE_COMMENT
%x BLOCK_COMMENT

SPACE     [ \t]
DIGIT     [0-9]
DIGITS    [0-9]+
HEXDIGIT  [0-9a-fA-F]
LETTER    [a-zA-Z]
SYMBOL    [+\-*/\\\^~=<>!?@#$%&|:`.]
TICK      "`"
ESCAPE    [nt\\'"]
CHARACTER [^"\\\x00-\x1f\x7f]|"\"{ESCAPE}|"\"{HEXDIGIT}{HEXDIGIT}
SIGN      [+-]?
NUM       {SIGN}{DIGIT}+
HEXNUM    {SIGN}"0x"{HEXDIGIT}+
INT       {NUM}|{HEXNUM}
FLOAT0    {NUM}"."{DIGIT}+
FLOAT1    {NUM}("."{DIGIT}+)?[eE]{NUM}
HEXFLOAT  {SIGN}"0x"{HEXDIGIT}+"."?{HEXDIGIT}*"p"{SIGN}{DIGIT}+
INFINITY  {SIGN}"infinity"
NAN       {SIGN}"nan"|{SIGN}"nan:0x"{HEXDIGIT}+
FLOAT     {FLOAT0}|{FLOAT1}|{HEXFLOAT}|{INFINITY}|{NAN}
TEXT      "\""{CHARACTER}*"\""
NAME      "$"({LETTER}|{DIGIT}|"_"|{TICK}|{SYMBOL})+

%%

"("                   { return WASM_TOKEN_TYPE_LPAR; }
")"                   { return WASM_TOKEN_TYPE_RPAR; }
{INT}                 { return WASM_TOKEN_TYPE_INT; }
{FLOAT}               { return WASM_TOKEN_TYPE_FLOAT; }
{TEXT}                { return WASM_TOKEN_TYPE_TEXT; }
"i32"                 { return WASM_TOKEN_TYPE_I32; }
"i64"                 { return WASM_TOKEN_TYPE_I64; }
"f32"                 { return WASM_TOKEN_TYPE_F32; }
"f64"                 { return WASM_TOKEN_TYPE_F64; }
"nop"                 { return WASM_TOKEN_TYPE_NOP; }
"block"               { return WASM_TOKEN_TYPE_BLOCK; }
"if"                  { return WASM_TOKEN_TYPE_IF; }
"if_else"             { return WASM_TOKEN_TYPE_IF_ELSE; }
"loop"                { return WASM_TOKEN_TYPE_LOOP; }
"label"               { return WASM_TOKEN_TYPE_LABEL; }
"br"                  { return WASM_TOKEN_TYPE_BR; }
"br_if"               { return WASM_TOKEN_TYPE_BR_IF; }
"tableswitch"         { return WASM_TOKEN_TYPE_TABLESWITCH; }
"case"                { return WASM_TOKEN_TYPE_CASE; }
"call"                { return WASM_TOKEN_TYPE_CALL; }
"call_import"         { return WASM_TOKEN_TYPE_CALL_IMPORT; }
"call_indirect"       { return WASM_TOKEN_TYPE_CALL_INDIRECT; }
"return"              { return WASM_TOKEN_TYPE_RETURN; }
"get_local"           { return WASM_TOKEN_TYPE_GET_LOCAL; }
"set_local"           { return WASM_TOKEN_TYPE_SET_LOCAL; }
"i32.load"            { return WASM_TOKEN_TYPE_I32_LOAD; }
"i64.load"            { return WASM_TOKEN_TYPE_I64_LOAD; }
"f32.load"            { return WASM_TOKEN_TYPE_F32_LOAD; }
"f64.load"            { return WASM_TOKEN_TYPE_F64_LOAD; }
"i32.store"           { return WASM_TOKEN_TYPE_I32_STORE; }
"i64.store"           { return WASM_TOKEN_TYPE_I64_STORE; }
"f32.store"           { return WASM_TOKEN_TYPE_F32_STORE; }
"f64.store"           { return WASM_TOKEN_TYPE_F64_STORE; }
"i32.load8_s"         { return WASM_TOKEN_TYPE_I32_LOAD8_S; }
"i64.load8_s"         { return WASM_TOKEN_TYPE_I64_LOAD8_S; }
"i32.load8_u"         { return WASM_TOKEN_TYPE_I32_LOAD8_U; }
"i64.load8_u"         { return WASM_TOKEN_TYPE_I64_LOAD8_U; }
"i32.load16_s"        { return WASM_TOKEN_TYPE_I32_LOAD16_S; }
"i64.load16_s"        { return WASM_TOKEN_TYPE_I64_LOAD16_S; }
"i32.load16_u"        { return WASM_TOKEN_TYPE_I32_LOAD16_U; }
"i64.load16_u"        { return WASM_TOKEN_TYPE_I64_LOAD16_U; }
"i32.store8"          { return WASM_TOKEN_TYPE_I32_STORE8; }
"i64.store8"          { return WASM_TOKEN_TYPE_I64_STORE8; }
"i32.store16"         { return WASM_TOKEN_TYPE_I32_STORE16; }
"i64.store16"         { return WASM_TOKEN_TYPE_I64_STORE16; }
"offset="{DIGITS}     { return WASM_TOKEN_TYPE_OFFSET; }
"align="{DIGITS}      { return WASM_TOKEN_TYPE_ALIGN; }
"i32.const"           { return WASM_TOKEN_TYPE_I32_CONST; }
"i64.const"           { return WASM_TOKEN_TYPE_I64_CONST; }
"f32.const"           { return WASM_TOKEN_TYPE_F32_CONST; }
"f64.const"           { return WASM_TOKEN_TYPE_F64_CONST; }
"i32.clz"             { return WASM_TOKEN_TYPE_I32_CLZ; }
"i64.clz"             { return WASM_TOKEN_TYPE_I64_CLZ; }
"i32.ctz"             { return WASM_TOKEN_TYPE_I32_CTZ; }
"i64.ctz"             { return WASM_TOKEN_TYPE_I64_CTZ; }
"i32.popcnt"          { return WASM_TOKEN_TYPE_I32_POPCNT; }
"i64.popcnt"          { return WASM_TOKEN_TYPE_I64_POPCNT; }
"f32.neg"             { return WASM_TOKEN_TYPE_F32_NEG; }
"f64.neg"             { return WASM_TOKEN_TYPE_F64_NEG; }
"f32.abs"             { return WASM_TOKEN_TYPE_F32_ABS; }
"f64.abs"             { return WASM_TOKEN_TYPE_F64_ABS; }
"f32.sqrt"            { return WASM_TOKEN_TYPE_F32_SQRT; }
"f64.sqrt"            { return WASM_TOKEN_TYPE_F64_SQRT; }
"f32.ceil"            { return WASM_TOKEN_TYPE_F32_CEIL; }
"f64.ceil"            { return WASM_TOKEN_TYPE_F64_CEIL; }
"f32.floor"           { return WASM_TOKEN_TYPE_F32_FLOOR; }
"f64.floor"           { return WASM_TOKEN_TYPE_F64_FLOOR; }
"f32.trunc"           { return WASM_TOKEN_TYPE_F32_TRUNC; }
"f64.trunc"           { return WASM_TOKEN_TYPE_F64_TRUNC; }
"f32.nearest"         { return WASM_TOKEN_TYPE_F32_NEAREST; }
"f64.nearest"         { return WASM_TOKEN_TYPE_F64_NEAREST; }
"i32.add"             { return WASM_TOKEN_TYPE_I32_ADD; }
"i64.add"             { return WASM_TOKEN_TYPE_I64_ADD; }
"i32.sub"             { return WASM_TOKEN_TYPE_I32_SUB; }
"i64.sub"             { return WASM_TOKEN_TYPE_I64_SUB; }
"i32.mul"             { return WASM_TOKEN_TYPE_I32_MUL; }
"i64.mul"             { return WASM_TOKEN_TYPE_I64_MUL; }
"i32.div_s"           { return WASM_TOKEN_TYPE_I32_DIV_S; }
"i64.div_s"           { return WASM_TOKEN_TYPE_I64_DIV_S; }
"i32.div_u"           { return WASM_TOKEN_TYPE_I32_DIV_U; }
"i64.div_u"           { return WASM_TOKEN_TYPE_I64_DIV_U; }
"i32.rem_s"           { return WASM_TOKEN_TYPE_I32_REM_S; }
"i64.rem_s"           { return WASM_TOKEN_TYPE_I64_REM_S; }
"i32.rem_u"           { return WASM_TOKEN_TYPE_I32_REM_U; }
"i64.rem_u"           { return WASM_TOKEN_TYPE_I64_REM_U; }
"i32.and"             { return WASM_TOKEN_TYPE_I32_AND; }
"i64.and"             { return WASM_TOKEN_TYPE_I64_AND; }
"i32.or"              { return WASM_TOKEN_TYPE_I32_OR; }
"i64.or"              { return WASM_TOKEN_TYPE_I64_OR; }
"i32.xor"             { return WASM_TOKEN_TYPE_I32_XOR; }
"i64.xor"             { return WASM_TOKEN_TYPE_I64_XOR; }
"i32.shl"             { return WASM_TOKEN_TYPE_I32_SHL; }
"i64.shl"             { return WASM_TOKEN_TYPE_I64_SHL; }
"i32.shr_s"           { return WASM_TOKEN_TYPE_I32_SHR_S; }
"i64.shr_s"           { return WASM_TOKEN_TYPE_I64_SHR_S; }
"i32.shr_u"           { return WASM_TOKEN_TYPE_I32_SHR_U; }
"i64.shr_u"           { return WASM_TOKEN_TYPE_I64_SHR_U; }
"f32.add"             { return WASM_TOKEN_TYPE_F32_ADD; }
"f64.add"             { return WASM_TOKEN_TYPE_F64_ADD; }
"f32.sub"             { return WASM_TOKEN_TYPE_F32_SUB; }
"f64.sub"             { return WASM_TOKEN_TYPE_F64_SUB; }
"f32.mul"             { return WASM_TOKEN_TYPE_F32_MUL; }
"f64.mul"             { return WASM_TOKEN_TYPE_F64_MUL; }
"f32.div"             { return WASM_TOKEN_TYPE_F32_DIV; }
"f64.div"             { return WASM_TOKEN_TYPE_F64_DIV; }
"f32.min"             { return WASM_TOKEN_TYPE_F32_MIN; }
"f64.min"             { return WASM_TOKEN_TYPE_F64_MIN; }
"f32.max"             { return WASM_TOKEN_TYPE_F32_MAX; }
"f64.max"             { return WASM_TOKEN_TYPE_F64_MAX; }
"f32.copysign"        { return WASM_TOKEN_TYPE_F32_COPYSIGN; }
"f64.copysign"        { return WASM_TOKEN_TYPE_F64_COPYSIGN; }
"i32.eq"              { return WASM_TOKEN_TYPE_I32_EQ; }
"i64.eq"              { return WASM_TOKEN_TYPE_I64_EQ; }
"i32.ne"              { return WASM_TOKEN_TYPE_I32_NE; }
"i64.ne"              { return WASM_TOKEN_TYPE_I64_NE; }
"i32.lt_s"            { return WASM_TOKEN_TYPE_I32_LT_S; }
"i64.lt_s"            { return WASM_TOKEN_TYPE_I64_LT_S; }
"i32.lt_u"            { return WASM_TOKEN_TYPE_I32_LT_U; }
"i64.lt_u"            { return WASM_TOKEN_TYPE_I64_LT_U; }
"i32.le_s"            { return WASM_TOKEN_TYPE_I32_LE_S; }
"i64.le_s"            { return WASM_TOKEN_TYPE_I64_LE_S; }
"i32.le_u"            { return WASM_TOKEN_TYPE_I32_LE_U; }
"i64.le_u"            { return WASM_TOKEN_TYPE_I64_LE_U; }
"i32.gt_s"            { return WASM_TOKEN_TYPE_I32_GT_S; }
"i64.gt_s"            { return WASM_TOKEN_TYPE_I64_GT_S; }
"i32.gt_u"            { return WASM_TOKEN_TYPE_I32_GT_U; }
"i64.gt_u"            { return WASM_TOKEN_TYPE_I64_GT_U; }
"i32.ge_s"            { return WASM_TOKEN_TYPE_I32_GE_S; }
"i64.ge_s"            { return WASM_TOKEN_TYPE_I64_GE_S; }
"i32.ge_u"            { return WASM_TOKEN_TYPE_I32_GE_U; }
"i64.ge_u"            { return WASM_TOKEN_TYPE_I64_GE_U; }
"f32.eq"              { return WASM_TOKEN_TYPE_F32_EQ; }
"f64.eq"              { return WASM_TOKEN_TYPE_F64_EQ; }
"f32.ne"              { return WASM_TOKEN_TYPE_F32_NE; }
"f64.ne"              { return WASM_TOKEN_TYPE_F64_NE; }
"f32.lt"              { return WASM_TOKEN_TYPE_F32_LT; }
"f64.lt"              { return WASM_TOKEN_TYPE_F64_LT; }
"f32.le"              { return WASM_TOKEN_TYPE_F32_LE; }
"f64.le"              { return WASM_TOKEN_TYPE_F64_LE; }
"f32.gt"              { return WASM_TOKEN_TYPE_F32_GT; }
"f64.gt"              { return WASM_TOKEN_TYPE_F64_GT; }
"f32.ge"              { return WASM_TOKEN_TYPE_F32_GE; }
"f64.ge"              { return WASM_TOKEN_TYPE_F64_GE; }
"i64.extend_s/i32"    { return WASM_TOKEN_TYPE_I64_EXTEND_S_I32; }
"i64.extend_u/i32"    { return WASM_TOKEN_TYPE_I64_EXTEND_U_I32; }
"i32.wrap/i64"        { return WASM_TOKEN_TYPE_I32_WRAP_I64; }
"i32.trunc_s/f32"     { return WASM_TOKEN_TYPE_I32_TRUNC_S_F32; }
"i64.trunc_s/f32"     { return WASM_TOKEN_TYPE_I64_TRUNC_U_F32; }
"i32.trunc_s/f64"     { return WASM_TOKEN_TYPE_I32_TRUNC_S_F64; }
"i64.trunc_s/f64"     { return WASM_TOKEN_TYPE_I64_TRUNC_U_F64; }
"i32.trunc_u/f32"     { return WASM_TOKEN_TYPE_I32_TRUNC_U_F32; }
"i64.trunc_u/f32"     { return WASM_TOKEN_TYPE_I64_TRUNC_U_F32; }
"i32.trunc_u/f64"     { return WASM_TOKEN_TYPE_I32_TRUNC_U_F64; }
"i64.trunc_u/f64"     { return WASM_TOKEN_TYPE_I64_TRUNC_U_F64; }
"f32.convert_s/i32"   { return WASM_TOKEN_TYPE_F32_CONVERT_S_I32; }
"f64.convert_s/i32"   { return WASM_TOKEN_TYPE_F64_CONVERT_U_I32; }
"f32.convert_s/i64"   { return WASM_TOKEN_TYPE_F32_CONVERT_S_I64; }
"f64.convert_s/i64"   { return WASM_TOKEN_TYPE_F64_CONVERT_U_I64; }
"f32.convert_u/i32"   { return WASM_TOKEN_TYPE_F32_CONVERT_U_I32; }
"f64.convert_u/i32"   { return WASM_TOKEN_TYPE_F64_CONVERT_U_I32; }
"f32.convert_u/i64"   { return WASM_TOKEN_TYPE_F32_CONVERT_U_I64; }
"f64.convert_u/i64"   { return WASM_TOKEN_TYPE_F64_CONVERT_U_I64; }
"f64.promote/f32"     { return WASM_TOKEN_TYPE_F64_PROMOTE_F32; }
"f32.demote/f64"      { return WASM_TOKEN_TYPE_F32_DEMOTE_F64; }
"f32.reinterpret/i32" { return WASM_TOKEN_TYPE_F32_REINTPRET_I32; }
"i32.reinterpret/f32" { return WASM_TOKEN_TYPE_I32_REINTPRET_F32; }
"f64.reinterpret/i64" { return WASM_TOKEN_TYPE_F64_REINTPRET_I64; }
"i64.reinterpret/f64" { return WASM_TOKEN_TYPE_I64_REINTPRET_F64; }
"i32.select"          { return WASM_TOKEN_TYPE_I32_SELECT; }
"i64.select"          { return WASM_TOKEN_TYPE_I64_SELECT; }
"f32.select"          { return WASM_TOKEN_TYPE_F32_SELECT; }
"f64.select"          { return WASM_TOKEN_TYPE_F64_SELECT; }
"unreachable"         { return WASM_TOKEN_TYPE_UNREACHABLE; }
"memory_size"         { return WASM_TOKEN_TYPE_MEMORY_SIZE; }
"grow_memory"         { return WASM_TOKEN_TYPE_GROW_MEMORY; }
"has_feature"         { return WASM_TOKEN_TYPE_HAS_FEATURE; }
"type"                { return WASM_TOKEN_TYPE_TYPE; }
"func"                { return WASM_TOKEN_TYPE_FUNC; }
"param"               { return WASM_TOKEN_TYPE_PARAM; }
"result"              { return WASM_TOKEN_TYPE_RESULT; }
"local"               { return WASM_TOKEN_TYPE_LOCAL; }
"module"              { return WASM_TOKEN_TYPE_MODULE; }
"memory"              { return WASM_TOKEN_TYPE_MEMORY; }
"segment"             { return WASM_TOKEN_TYPE_SEGMENT; }
"import"              { return WASM_TOKEN_TYPE_IMPORT; }
"export"              { return WASM_TOKEN_TYPE_EXPORT; }
"table"               { return WASM_TOKEN_TYPE_TABLE; }
"assert_invalid"      { return WASM_TOKEN_TYPE_ASSERT_INVALID; }
"assert_return"       { return WASM_TOKEN_TYPE_ASSERT_RETURN; }
"assert_return_nan"   { return WASM_TOKEN_TYPE_ASSERT_RETURN_NAN; }
"assert_trap"         { return WASM_TOKEN_TYPE_ASSERT_TRAP; }
"invoke"              { return WASM_TOKEN_TYPE_INVOKE; }
{NAME}                { return WASM_TOKEN_TYPE_NAME; }

";;"                  { BEGIN(LINE_COMMENT); }
<LINE_COMMENT>\n      { COLUMN(yyscanner) = 1; BEGIN(INITIAL); }
<LINE_COMMENT><<EOF>> { return WASM_TOKEN_TYPE_EOF; }
<LINE_COMMENT>.
"(;"                  { BEGIN(BLOCK_COMMENT); COMMENT_NESTING(yyscanner) = 1; }
<BLOCK_COMMENT>"(;"   { COMMENT_NESTING(yyscanner)++; }
<BLOCK_COMMENT>";)"   { if (--COMMENT_NESTING(yyscanner) == 0) BEGIN(INITIAL); }
<BLOCK_COMMENT>\n     { COLUMN(yyscanner) = 1; }
<BLOCK_COMMENT><<EOF>> { return WASM_TOKEN_TYPE_ERROR; }
<BLOCK_COMMENT>.
\n                    { COLUMN(yyscanner) = 1; }
[ \t\r]
<<EOF>>               { return WASM_TOKEN_TYPE_EOF; }
.                     { return WASM_TOKEN_TYPE_ERROR; }

%%

static const char* s_token_names[] = {
#define V(name) #name,
  WASM_TOKEN_TYPES(V)
#undef V
};

int main(int argc, char **argv) {
  ++argv, --argc; /* skip over program name */
  yyscan_t scanner;
  WasmScannerExtra extra = {};
  extra.column = 1;

  yylex_init(&scanner);
  if (argc > 0)
    yyset_in(fopen(argv[0], "r"), scanner);
  else
    yyset_in(stdin, scanner);
  yyset_extra(&extra, scanner);
  while (1) {
    WasmToken token;
    WasmLocation location;
    WasmTokenType type = yylex(&token, &location, scanner);
    if (type == WASM_TOKEN_TYPE_EOF || type == WASM_TOKEN_TYPE_ERROR)
      break;
    printf("%d:%d:%s \"%.*s\"\n", location.first_line, location.first_column,
           s_token_names[type], (int)(token.end - token.start), token.start);
  }
  yylex_destroy(scanner);
}

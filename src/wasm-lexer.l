%{
#include "wasm.h"
#include "wasm-internal.h"
#include "wasm-parser.h"

typedef struct WasmScannerExtra {
  FILE* file;
  const char* filename;
  int column;
  int comment_nesting;
} WasmScannerExtra;

#define YY_DECL                                                               \
  int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, yyscan_t yyscanner, \
            WasmParser* parser)

#define EXTRA(scanner) ((WasmScannerExtra*)yyget_extra(scanner))
#define COMMENT_NESTING(scanner) (EXTRA(scanner)->comment_nesting)
#define COLUMN(scanner) (EXTRA(scanner)->column)
#define RESET_COLUMN(scanner) EXTRA(scanner)->column = 1
#define YY_USER_ACTION                                 \
  {                                                    \
    WasmScannerExtra* extra = EXTRA(yyscanner);        \
    yylloc->filename = extra->filename;                \
    yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = extra->column;              \
    extra->column += yyleng;                           \
    yylloc->last_column = extra->column;               \
  }

#define TEXT                   \
  yylval->text.start = yytext; \
  yylval->text.length = yyleng
#define TEXT_AT(offset)                 \
  yylval->text.start = yytext + offset; \
  yylval->text.length = yyleng - offset
#define TYPE(type_) yylval->type = WASM_TYPE_##type_
#define TYPE_STRUCT(struct_, type_) yylval->struct_.type = WASM_TYPE_##type_
#define SIGN(struct_, sign_) \
  yylval->struct_.sign = sign_ ? WASM_SIGNED : WASM_UNSIGNED
#define MEMOP(type_, size_) \
  TYPE_STRUCT(mem, type_);  \
  yylval->mem.size = WASM_MEM_SIZE_##size_
#define MEMOPSIGN(type_, size_, sign_) \
  MEMOP(type_, size_);                 \
  SIGN(mem, sign_)
#define UNOP(type_, name)    \
  TYPE_STRUCT(unary, type_); \
  yylval->unary.op_type = WASM_UNARY_OP_TYPE_##name
#define BINOP(type_, name)    \
  TYPE_STRUCT(binary, type_); \
  yylval->binary.op_type = WASM_BINARY_OP_TYPE_##name
#define BINOPSIGN(type_, name, sign_) \
  BINOP(type_, name);                 \
  SIGN(binary, sign_)
#define CMPOP(type_, name)     \
  TYPE_STRUCT(compare, type_); \
  yylval->compare.op_type = WASM_COMPARE_OP_TYPE_##name
#define CMPOPSIGN(type_, name, sign_) \
  CMPOP(type_, name);                 \
  SIGN(compare, sign_)
#define CONVTYPE(type_, type2_) \
  TYPE_STRUCT(convert, type_);  \
  yylval->convert.type2 = WASM_TYPE_##type2_
#define CONVTYPESIGN(type_, type2_, sign_) \
  CONVTYPE(type_, type2_);                 \
  SIGN(convert, sign_)
#define CASTTYPE(type_, type2_) \
  TYPE_STRUCT(cast, type_);     \
  yylval->cast.type2 = WASM_TYPE_##type2_

#define TOK(name) WASM_TOKEN_TYPE_##name

%}

%option bison-bridge
%option bison-locations
%option yylineno
%option reentrant

%x LINE_COMMENT
%x BLOCK_COMMENT
%x BAD_TEXT

SPACE     [ \t]
DIGIT     [0-9]
DIGITS    [0-9]+
HEXDIGIT  [0-9a-fA-F]
LETTER    [a-zA-Z]
SYMBOL    [+\-*/\\\^~=<>!?@#$%&|:`.]
TICK      `
ESCAPE    [nt\\'"]
CHARACTER [^"\\\x00-\x1f\x7f]|\\{ESCAPE}|\\{HEXDIGIT}{HEXDIGIT}
SIGN      [+-]?
NUM       {SIGN}{DIGIT}+
HEXNUM    {SIGN}"0x"{HEXDIGIT}+
INT       {NUM}|{HEXNUM}
FLOAT0    {NUM}\.{DIGIT}+
FLOAT1    {NUM}(\.{DIGIT}+)?[eE]{NUM}
HEXFLOAT  {SIGN}"0x"{HEXDIGIT}+\.?{HEXDIGIT}*p{SIGN}{DIGIT}+
INFINITY  {SIGN}"infinity"
NAN       {SIGN}"nan"|{SIGN}"nan:0x"{HEXDIGIT}+
FLOAT     {FLOAT0}|{FLOAT1}|{HEXFLOAT}|{INFINITY}|{NAN}
TEXT      \"{CHARACTER}*\"
ATOM      ({LETTER}|{DIGIT}|_|{TICK}|{SYMBOL})+
NAME      \${ATOM}

%%

"("                   { return TOK(LPAR); }
")"                   { return TOK(RPAR); }
{INT}                 { TEXT; return TOK(INT); }
{FLOAT}               { TEXT; return TOK(FLOAT); }
{TEXT}                { TEXT; return TOK(TEXT); }
\"                    { BEGIN(BAD_TEXT); }
<BAD_TEXT>{CHARACTER} {}
<BAD_TEXT>\n          { BEGIN(INITIAL);
                        RESET_COLUMN(yyscanner);
                        yyerror(yylloc, &yyscanner, parser,
                                "newline in string"); }
<BAD_TEXT><<EOF>>     { yyerror(yylloc, &yyscanner, parser, "unexpected EOF");
                        return TOK(EOF); }
<BAD_TEXT>\\.         { yyerror(yylloc, &yyscanner, parser,
                                "bad escape \"%.*s\"", yyleng, yytext); }
<BAD_TEXT>\"          { BEGIN(INITIAL); TEXT; return TOK(TEXT); }
<BAD_TEXT>[^\\]       { yyerror(yylloc, &yyscanner, parser,
                                "illegal character in string"); }
"i32"                 { TYPE(I32); return TOK(VALUE_TYPE); }
"i64"                 { TYPE(I64); return TOK(VALUE_TYPE); }
"f32"                 { TYPE(F32); return TOK(VALUE_TYPE); }
"f64"                 { TYPE(F64); return TOK(VALUE_TYPE); }
"nop"                 { return TOK(NOP); }
"block"               { return TOK(BLOCK); }
"if"                  { return TOK(IF); }
"if_else"             { return TOK(IF_ELSE); }
"loop"                { return TOK(LOOP); }
"label"               { return TOK(LABEL); }
"br"                  { return TOK(BR); }
"br_if"               { return TOK(BR_IF); }
"tableswitch"         { return TOK(TABLESWITCH); }
"case"                { return TOK(CASE); }
"call"                { return TOK(CALL); }
"call_import"         { return TOK(CALL_IMPORT); }
"call_indirect"       { return TOK(CALL_INDIRECT); }
"return"              { return TOK(RETURN); }
"get_local"           { return TOK(GET_LOCAL); }
"set_local"           { return TOK(SET_LOCAL); }
"i32.load"            { MEMOP(I32, 32); return TOK(LOAD); }
"i64.load"            { MEMOP(I64, 64); return TOK(LOAD); }
"f32.load"            { MEMOP(F32, 32); return TOK(LOAD); }
"f64.load"            { MEMOP(F64, 64); return TOK(LOAD); }
"i32.store"           { MEMOP(I32, 32); return TOK(STORE); }
"i64.store"           { MEMOP(I64, 64); return TOK(STORE); }
"f32.store"           { MEMOP(F32, 32); return TOK(STORE); }
"f64.store"           { MEMOP(F64, 64); return TOK(STORE); }
"i32.load8_s"         { MEMOPSIGN(I32, 8, 1); return TOK(LOAD_EXTEND); }
"i64.load8_s"         { MEMOPSIGN(I64, 8, 1); return TOK(LOAD_EXTEND); }
"i32.load8_u"         { MEMOPSIGN(I32, 8, 0); return TOK(LOAD_EXTEND); }
"i64.load8_u"         { MEMOPSIGN(I64, 8, 0); return TOK(LOAD_EXTEND); }
"i32.load16_s"        { MEMOPSIGN(I32, 16, 1); return TOK(LOAD_EXTEND); }
"i64.load16_s"        { MEMOPSIGN(I64, 16, 1); return TOK(LOAD_EXTEND); }
"i32.load16_u"        { MEMOPSIGN(I32, 16, 0); return TOK(LOAD_EXTEND); }
"i64.load16_u"        { MEMOPSIGN(I64, 16, 0); return TOK(LOAD_EXTEND); }
"i64.load32_s"        { MEMOPSIGN(I64, 32, 1); return TOK(LOAD_EXTEND); }
"i64.load32_u"        { MEMOPSIGN(I64, 32, 0); return TOK(LOAD_EXTEND); }
"i32.store8"          { MEMOP(I32, 8); return TOK(STORE_WRAP); }
"i64.store8"          { MEMOP(I64, 8); return TOK(STORE_WRAP); }
"i32.store16"         { MEMOP(I32, 16); return TOK(STORE_WRAP); }
"i64.store16"         { MEMOP(I64, 16); return TOK(STORE_WRAP); }
"i64.store32"         { MEMOP(I64, 32); return TOK(STORE_WRAP); }
"offset="{DIGITS}     { TEXT_AT(7); return TOK(OFFSET); }
"align="{DIGITS}      { TEXT_AT(6); return TOK(ALIGN); }
"i32.const"           { TYPE(I32); return TOK(CONST); }
"i64.const"           { TYPE(I64); return TOK(CONST); }
"f32.const"           { TYPE(F32); return TOK(CONST); }
"f64.const"           { TYPE(F64); return TOK(CONST); }
"i32.not"             { UNOP(I32, NOT); return TOK(UNARY); }
"i32.clz"             { UNOP(I32, CLZ); return TOK(UNARY); }
"i64.clz"             { UNOP(I64, CLZ); return TOK(UNARY); }
"i32.ctz"             { UNOP(I32, CTZ); return TOK(UNARY); }
"i64.ctz"             { UNOP(I64, CTZ); return TOK(UNARY); }
"i32.popcnt"          { UNOP(I32, POPCNT); return TOK(UNARY); }
"i64.popcnt"          { UNOP(I64, POPCNT); return TOK(UNARY); }
"f32.neg"             { UNOP(F32, NEG); return TOK(UNARY); }
"f64.neg"             { UNOP(F64, NEG); return TOK(UNARY); }
"f32.abs"             { UNOP(F32, ABS); return TOK(UNARY); }
"f64.abs"             { UNOP(F64, ABS); return TOK(UNARY); }
"f32.sqrt"            { UNOP(F32, SQRT); return TOK(UNARY); }
"f64.sqrt"            { UNOP(F64, SQRT); return TOK(UNARY); }
"f32.ceil"            { UNOP(F32, CEIL); return TOK(UNARY); }
"f64.ceil"            { UNOP(F64, CEIL); return TOK(UNARY); }
"f32.floor"           { UNOP(F32, FLOOR); return TOK(UNARY); }
"f64.floor"           { UNOP(F64, FLOOR); return TOK(UNARY); }
"f32.trunc"           { UNOP(F32, TRUNC); return TOK(UNARY); }
"f64.trunc"           { UNOP(F64, TRUNC); return TOK(UNARY); }
"f32.nearest"         { UNOP(F32, NEAREST); return TOK(UNARY); }
"f64.nearest"         { UNOP(F64, NEAREST); return TOK(UNARY); }
"i32.add"             { BINOP(I32, ADD); return TOK(BINARY); }
"i64.add"             { BINOP(I64, ADD); return TOK(BINARY); }
"i32.sub"             { BINOP(I32, SUB); return TOK(BINARY); }
"i64.sub"             { BINOP(I64, SUB); return TOK(BINARY); }
"i32.mul"             { BINOP(I32, MUL); return TOK(BINARY); }
"i64.mul"             { BINOP(I64, MUL); return TOK(BINARY); }
"i32.div_s"           { BINOPSIGN(I32, DIV, 1); return TOK(BINARY); }
"i64.div_s"           { BINOPSIGN(I64, DIV, 1); return TOK(BINARY); }
"i32.div_u"           { BINOPSIGN(I32, DIV, 0); return TOK(BINARY); }
"i64.div_u"           { BINOPSIGN(I64, DIV, 0); return TOK(BINARY); }
"i32.rem_s"           { BINOPSIGN(I32, REM, 1); return TOK(BINARY); }
"i64.rem_s"           { BINOPSIGN(I64, REM, 1); return TOK(BINARY); }
"i32.rem_u"           { BINOPSIGN(I32, REM, 0); return TOK(BINARY); }
"i64.rem_u"           { BINOPSIGN(I64, REM, 0); return TOK(BINARY); }
"i32.and"             { BINOP(I32, AND); return TOK(BINARY); }
"i64.and"             { BINOP(I64, AND); return TOK(BINARY); }
"i32.or"              { BINOP(I32, OR); return TOK(BINARY); }
"i64.or"              { BINOP(I64, OR); return TOK(BINARY); }
"i32.xor"             { BINOP(I32, XOR); return TOK(BINARY); }
"i64.xor"             { BINOP(I64, XOR); return TOK(BINARY); }
"i32.shl"             { BINOP(I32, SHL); return TOK(BINARY); }
"i64.shl"             { BINOP(I64, SHL); return TOK(BINARY); }
"i32.shr_s"           { BINOPSIGN(I32, SHR, 1); return TOK(BINARY); }
"i64.shr_s"           { BINOPSIGN(I64, SHR, 1); return TOK(BINARY); }
"i32.shr_u"           { BINOPSIGN(I32, SHR, 0); return TOK(BINARY); }
"i64.shr_u"           { BINOPSIGN(I64, SHR, 0); return TOK(BINARY); }
"f32.add"             { BINOP(F32, ADD); return TOK(BINARY); }
"f64.add"             { BINOP(F64, ADD); return TOK(BINARY); }
"f32.sub"             { BINOP(F32, SUB); return TOK(BINARY); }
"f64.sub"             { BINOP(F64, SUB); return TOK(BINARY); }
"f32.mul"             { BINOP(F32, MUL); return TOK(BINARY); }
"f64.mul"             { BINOP(F64, MUL); return TOK(BINARY); }
"f32.div"             { BINOP(F32, DIV); return TOK(BINARY); }
"f64.div"             { BINOP(F64, DIV); return TOK(BINARY); }
"f32.min"             { BINOP(F32, MIN); return TOK(BINARY); }
"f64.min"             { BINOP(F64, MIN); return TOK(BINARY); }
"f32.max"             { BINOP(F32, MAX); return TOK(BINARY); }
"f64.max"             { BINOP(F64, MAX); return TOK(BINARY); }
"f32.copysign"        { BINOP(F32, COPYSIGN); return TOK(BINARY); }
"f64.copysign"        { BINOP(F64, COPYSIGN); return TOK(BINARY); }
"i32.eq"              { CMPOP(I32, EQ); return TOK(COMPARE); }
"i64.eq"              { CMPOP(I64, EQ); return TOK(COMPARE); }
"i32.ne"              { CMPOP(I32, EQ); return TOK(COMPARE); }
"i64.ne"              { CMPOP(I64, EQ); return TOK(COMPARE); }
"i32.lt_s"            { CMPOPSIGN(I32, LT, 1); return TOK(COMPARE); }
"i64.lt_s"            { CMPOPSIGN(I64, LT, 1); return TOK(COMPARE); }
"i32.lt_u"            { CMPOPSIGN(I32, LT, 0); return TOK(COMPARE); }
"i64.lt_u"            { CMPOPSIGN(I64, LT, 0); return TOK(COMPARE); }
"i32.le_s"            { CMPOPSIGN(I32, LE, 1); return TOK(COMPARE); }
"i64.le_s"            { CMPOPSIGN(I64, LE, 1); return TOK(COMPARE); }
"i32.le_u"            { CMPOPSIGN(I32, LE, 0); return TOK(COMPARE); }
"i64.le_u"            { CMPOPSIGN(I64, LE, 0); return TOK(COMPARE); }
"i32.gt_s"            { CMPOPSIGN(I32, GT, 1); return TOK(COMPARE); }
"i64.gt_s"            { CMPOPSIGN(I64, GT, 1); return TOK(COMPARE); }
"i32.gt_u"            { CMPOPSIGN(I32, GT, 0); return TOK(COMPARE); }
"i64.gt_u"            { CMPOPSIGN(I64, GT, 0); return TOK(COMPARE); }
"i32.ge_s"            { CMPOPSIGN(I32, GE, 1); return TOK(COMPARE); }
"i64.ge_s"            { CMPOPSIGN(I64, GE, 1); return TOK(COMPARE); }
"i32.ge_u"            { CMPOPSIGN(I32, GE, 0); return TOK(COMPARE); }
"i64.ge_u"            { CMPOPSIGN(I64, GE, 0); return TOK(COMPARE); }
"f32.eq"              { CMPOP(F32, EQ); return TOK(COMPARE); }
"f64.eq"              { CMPOP(F64, EQ); return TOK(COMPARE); }
"f32.ne"              { CMPOP(F32, NE); return TOK(COMPARE); }
"f64.ne"              { CMPOP(F64, NE); return TOK(COMPARE); }
"f32.lt"              { CMPOP(F32, LT); return TOK(COMPARE); }
"f64.lt"              { CMPOP(F64, LT); return TOK(COMPARE); }
"f32.le"              { CMPOP(F32, LE); return TOK(COMPARE); }
"f64.le"              { CMPOP(F64, LE); return TOK(COMPARE); }
"f32.gt"              { CMPOP(F32, GT); return TOK(COMPARE); }
"f64.gt"              { CMPOP(F64, GT); return TOK(COMPARE); }
"f32.ge"              { CMPOP(F32, GE); return TOK(COMPARE); }
"f64.ge"              { CMPOP(F64, GE); return TOK(COMPARE); }
"i64.extend_s/i32"    { CONVTYPESIGN(I64, I32, 1); return TOK(CONVERT); }
"i64.extend_u/i32"    { CONVTYPESIGN(I64, I32, 0); return TOK(CONVERT); }
"i32.wrap/i64"        { CONVTYPE(I32, I64); return TOK(CONVERT); }
"i32.trunc_s/f32"     { CONVTYPESIGN(I32, F32, 1); return TOK(CONVERT); }
"i64.trunc_s/f32"     { CONVTYPESIGN(I64, F32, 1); return TOK(CONVERT); }
"i32.trunc_s/f64"     { CONVTYPESIGN(I32, F64, 1); return TOK(CONVERT); }
"i64.trunc_s/f64"     { CONVTYPESIGN(I64, F64, 1); return TOK(CONVERT); }
"i32.trunc_u/f32"     { CONVTYPESIGN(I32, F32, 0); return TOK(CONVERT); }
"i64.trunc_u/f32"     { CONVTYPESIGN(I64, F32, 0); return TOK(CONVERT); }
"i32.trunc_u/f64"     { CONVTYPESIGN(I32, F64, 0); return TOK(CONVERT); }
"i64.trunc_u/f64"     { CONVTYPESIGN(I64, F64, 0); return TOK(CONVERT); }
"f32.convert_s/i32"   { CONVTYPESIGN(F32, I32, 1); return TOK(CONVERT); }
"f64.convert_s/i32"   { CONVTYPESIGN(F64, I32, 1); return TOK(CONVERT); }
"f32.convert_s/i64"   { CONVTYPESIGN(F32, I64, 1); return TOK(CONVERT); }
"f64.convert_s/i64"   { CONVTYPESIGN(F64, I64, 1); return TOK(CONVERT); }
"f32.convert_u/i32"   { CONVTYPESIGN(F32, I32, 0); return TOK(CONVERT); }
"f64.convert_u/i32"   { CONVTYPESIGN(F64, I32, 0); return TOK(CONVERT); }
"f32.convert_u/i64"   { CONVTYPESIGN(F32, I64, 0); return TOK(CONVERT); }
"f64.convert_u/i64"   { CONVTYPESIGN(F64, I64, 0); return TOK(CONVERT); }
"f64.promote/f32"     { CONVTYPE(F64, F32); return TOK(CONVERT); }
"f32.demote/f64"      { CONVTYPE(F32, F64); return TOK(CONVERT); }
"f32.reinterpret/i32" { CASTTYPE(F32, I32); return TOK(CAST); }
"i32.reinterpret/f32" { CASTTYPE(I32, F32); return TOK(CAST); }
"f64.reinterpret/i64" { CASTTYPE(F64, I64); return TOK(CAST); }
"i64.reinterpret/f64" { CASTTYPE(I64, F64); return TOK(CAST); }
"i32.select"          { TYPE(I32); return TOK(SELECT); }
"i64.select"          { TYPE(I64); return TOK(SELECT); }
"f32.select"          { TYPE(F32); return TOK(SELECT); }
"f64.select"          { TYPE(F64); return TOK(SELECT); }
"unreachable"         { return TOK(UNREACHABLE); }
"memory_size"         { return TOK(MEMORY_SIZE); }
"grow_memory"         { return TOK(GROW_MEMORY); }
"has_feature"         { return TOK(HAS_FEATURE); }
"type"                { return TOK(TYPE); }
"func"                { return TOK(FUNC); }
"param"               { return TOK(PARAM); }
"result"              { return TOK(RESULT); }
"local"               { return TOK(LOCAL); }
"module"              { return TOK(MODULE); }
"memory"              { return TOK(MEMORY); }
"segment"             { return TOK(SEGMENT); }
"import"              { return TOK(IMPORT); }
"export"              { return TOK(EXPORT); }
"table"               { return TOK(TABLE); }
"assert_invalid"      { return TOK(ASSERT_INVALID); }
"assert_return"       { return TOK(ASSERT_RETURN); }
"assert_return_nan"   { return TOK(ASSERT_RETURN_NAN); }
"assert_trap"         { return TOK(ASSERT_TRAP); }
"invoke"              { return TOK(INVOKE); }
{NAME}                { TEXT; return TOK(VAR); }

"break"               { return TOK(BR); }
"global"              { return TOK(GLOBAL); }
"load_global"         { return TOK(LOAD_GLOBAL); }
"store_global"        { return TOK(STORE_GLOBAL); }
"page_size"           { return TOK(PAGE_SIZE); }

";;"                  { BEGIN(LINE_COMMENT); }
<LINE_COMMENT>\n      { RESET_COLUMN(yyscanner); BEGIN(INITIAL); }
<LINE_COMMENT><<EOF>> { return TOK(EOF); }
<LINE_COMMENT>.
"(;"                  { BEGIN(BLOCK_COMMENT); COMMENT_NESTING(yyscanner) = 1; }
<BLOCK_COMMENT>"(;"   { COMMENT_NESTING(yyscanner)++; }
<BLOCK_COMMENT>";)"   { if (--COMMENT_NESTING(yyscanner) == 0) BEGIN(INITIAL); }
<BLOCK_COMMENT>\n     { RESET_COLUMN(yyscanner); }
<BLOCK_COMMENT><<EOF>> { yyerror(yylloc, &yyscanner, parser,
                                 "unexpected EOF");
                         return TOK(EOF); }
<BLOCK_COMMENT>.
\n                    { RESET_COLUMN(yyscanner); }
[ \t\r]
<<EOF>>               { return TOK(EOF); }
{ATOM}                { yyerror(yylloc, &yyscanner, parser,
                                "unexpected token \"%.*s\"", yyleng, yytext); }
.                     { yyerror(yylloc, &yyscanner, parser,
                                "unexpected char"); }

%%

WasmScanner wasm_new_scanner(const char* filename) {
  WasmScannerExtra* extra = malloc(sizeof(WasmScannerExtra));
  extra->column = 1;
  extra->filename = filename;
  extra->file = fopen(filename, "r");

  if (!extra->file) {
    free(extra);
    return NULL;
  }

  yyscan_t scanner;
  yylex_init(&scanner);
  yyset_in(extra->file, scanner);
  yyset_extra(extra, scanner);
  return scanner;
}

void wasm_free_scanner(WasmScanner scanner) {
  WasmScannerExtra* extra = yyget_extra(scanner);
  fclose(extra->file);
  free(extra);
  yylex_destroy((yyscan_t)scanner);
}

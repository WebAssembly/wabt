/* C code produced by gperf version 3.0.4 */
/* Command-line: gperf --compare-strncmp --readonly-tables --struct-type --output-file src/wasm-keywords.h src/wasm-keywords.gperf  */
/* Computed positions: -k'1-2,5-6,9,11-13,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "src/wasm-keywords.gperf"
struct OpInfo {
  const char* name;
  int op_type;
  int opcode;
  int type;
  int type2;
  int access;
};

#define TOTAL_KEYWORDS 183
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 18
#define MAX_HASH_VALUE 432
/* maximum key range = 415, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned short asso_values[] =
    {
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433,  30, 433,  10,
       90,   5,  20, 433,   0, 433,  35, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433,  15, 433,  10,  25, 115,
        5,  15,  10,   0,  80,   0, 433,  50,  30, 140,
       50, 105,  15,  60,   5,   5,   5,  55,  25,  20,
      150,  55,   5, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433, 433, 433, 433, 433,
      433, 433, 433, 433, 433, 433
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[12]];
      /*FALLTHROUGH*/
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      /*FALLTHROUGH*/
      case 11:
        hval += asso_values[(unsigned char)str[10]];
      /*FALLTHROUGH*/
      case 10:
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
      case 7:
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
      case 3:
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct OpInfo *
in_word_set (str, len)
     register const char *str;
     register unsigned int len;
{
  static const struct OpInfo wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 139 "src/wasm-keywords.gperf"
      {"i64.gt_s", WASM_OP_COMPARE, WASM_OPCODE_I64_SGT, WASM_TYPE_I64},
      {""}, {""}, {""},
#line 173 "src/wasm-keywords.gperf"
      {"if", WASM_OP_IF, WASM_OPCODE_IF},
#line 94 "src/wasm-keywords.gperf"
      {"i32.gt_s", WASM_OP_COMPARE, WASM_OPCODE_I32_SGT, WASM_TYPE_I32},
#line 132 "src/wasm-keywords.gperf"
      {"i64.div_s", WASM_OP_BINARY, WASM_OPCODE_I64_SDIV, WASM_TYPE_I64},
      {""},
#line 64 "src/wasm-keywords.gperf"
      {"f64.gt", WASM_OP_COMPARE, WASM_OPCODE_F64_GT, WASM_TYPE_F64},
#line 127 "src/wasm-keywords.gperf"
      {"i64.add", WASM_OP_BINARY, WASM_OPCODE_I64_ADD, WASM_TYPE_I64},
#line 137 "src/wasm-keywords.gperf"
      {"i64.ge_s", WASM_OP_COMPARE, WASM_OPCODE_I64_SGE, WASM_TYPE_I64},
#line 89 "src/wasm-keywords.gperf"
      {"i32.div_s", WASM_OP_BINARY, WASM_OPCODE_I32_SDIV, WASM_TYPE_I32},
      {""},
#line 35 "src/wasm-keywords.gperf"
      {"f32.gt", WASM_OP_COMPARE, WASM_OPCODE_F32_GT, WASM_TYPE_F32},
#line 84 "src/wasm-keywords.gperf"
      {"i32.add", WASM_OP_BINARY, WASM_OPCODE_I32_ADD, WASM_TYPE_I32},
#line 92 "src/wasm-keywords.gperf"
      {"i32.ge_s", WASM_OP_COMPARE, WASM_OPCODE_I32_SGE, WASM_TYPE_I32},
      {""}, {""},
#line 162 "src/wasm-keywords.gperf"
      {"i64.store16", WASM_OP_STORE, WASM_OPCODE_I64_STORE_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I16, 5},
#line 52 "src/wasm-keywords.gperf"
      {"f64.add", WASM_OP_BINARY, WASM_OPCODE_F64_ADD, WASM_TYPE_F64},
      {""},
#line 157 "src/wasm-keywords.gperf"
      {"i64.rem_s", WASM_OP_BINARY, WASM_OPCODE_I64_SREM, WASM_TYPE_I64},
      {""},
#line 116 "src/wasm-keywords.gperf"
      {"i32.store16", WASM_OP_STORE, WASM_OPCODE_I32_STORE_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I16, 5},
#line 22 "src/wasm-keywords.gperf"
      {"f32.add", WASM_OP_BINARY, WASM_OPCODE_F32_ADD, WASM_TYPE_F32},
      {""},
#line 111 "src/wasm-keywords.gperf"
      {"i32.rem_s", WASM_OP_BINARY, WASM_OPCODE_I32_SREM, WASM_TYPE_I32},
      {""},
#line 63 "src/wasm-keywords.gperf"
      {"f64.ge", WASM_OP_COMPARE, WASM_OPCODE_F64_GE, WASM_TYPE_F64},
#line 60 "src/wasm-keywords.gperf"
      {"f64.div", WASM_OP_BINARY, WASM_OPCODE_F64_DIV, WASM_TYPE_F64},
#line 150 "src/wasm-keywords.gperf"
      {"i64.lt_s", WASM_OP_COMPARE, WASM_OPCODE_I64_SLT, WASM_TYPE_I64},
#line 165 "src/wasm-keywords.gperf"
      {"i64.store", WASM_OP_STORE, WASM_OPCODE_I64_STORE_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I64, 7},
#line 191 "src/wasm-keywords.gperf"
      {"table", WASM_OP_TABLE},
#line 34 "src/wasm-keywords.gperf"
      {"f32.ge", WASM_OP_COMPARE, WASM_OPCODE_F32_GE, WASM_TYPE_F32},
#line 31 "src/wasm-keywords.gperf"
      {"f32.div", WASM_OP_BINARY, WASM_OPCODE_F32_DIV, WASM_TYPE_F32},
#line 103 "src/wasm-keywords.gperf"
      {"i32.lt_s", WASM_OP_COMPARE, WASM_OPCODE_I32_SLT, WASM_TYPE_I32},
#line 118 "src/wasm-keywords.gperf"
      {"i32.store", WASM_OP_STORE, WASM_OPCODE_I32_STORE_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I32, 6},
#line 16 "src/wasm-keywords.gperf"
      {"br_if", WASM_OP_BR_IF},
#line 67 "src/wasm-keywords.gperf"
      {"f64.lt", WASM_OP_COMPARE, WASM_OPCODE_F64_LT, WASM_TYPE_F64},
#line 51 "src/wasm-keywords.gperf"
      {"f64.abs", WASM_OP_UNARY, WASM_OPCODE_F64_ABS, WASM_TYPE_F64},
#line 141 "src/wasm-keywords.gperf"
      {"i64.le_s", WASM_OP_COMPARE, WASM_OPCODE_I64_SLE, WASM_TYPE_I64},
#line 77 "src/wasm-keywords.gperf"
      {"f64.store", WASM_OP_STORE, WASM_OPCODE_F64_STORE_I32, WASM_TYPE_F64, WASM_MEM_TYPE_F64, 0},
      {""},
#line 38 "src/wasm-keywords.gperf"
      {"f32.lt", WASM_OP_COMPARE, WASM_OPCODE_F32_LT, WASM_TYPE_F32},
#line 21 "src/wasm-keywords.gperf"
      {"f32.abs", WASM_OP_UNARY, WASM_OPCODE_F32_ABS, WASM_TYPE_F32},
#line 96 "src/wasm-keywords.gperf"
      {"i32.le_s", WASM_OP_COMPARE, WASM_OPCODE_I32_SLE, WASM_TYPE_I32},
#line 47 "src/wasm-keywords.gperf"
      {"f32.store", WASM_OP_STORE, WASM_OPCODE_F32_STORE_I32, WASM_TYPE_F32, WASM_MEM_TYPE_F32, 0},
      {""},
#line 187 "src/wasm-keywords.gperf"
      {"result", WASM_OP_RESULT},
      {""},
#line 140 "src/wasm-keywords.gperf"
      {"i64.gt_u", WASM_OP_COMPARE, WASM_OPCODE_I64_UGT, WASM_TYPE_I64},
#line 62 "src/wasm-keywords.gperf"
      {"f64.floor", WASM_OP_UNARY, WASM_OPCODE_F64_FLOOR, WASM_TYPE_F64},
#line 164 "src/wasm-keywords.gperf"
      {"i64.store8", WASM_OP_STORE, WASM_OPCODE_I64_STORE_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I8, 4},
#line 13 "src/wasm-keywords.gperf"
      {"assert_trap", WASM_OP_ASSERT_TRAP},
#line 128 "src/wasm-keywords.gperf"
      {"i64.and", WASM_OP_BINARY, WASM_OPCODE_I64_AND, WASM_TYPE_I64},
#line 95 "src/wasm-keywords.gperf"
      {"i32.gt_u", WASM_OP_COMPARE, WASM_OPCODE_I32_UGT, WASM_TYPE_I32},
#line 33 "src/wasm-keywords.gperf"
      {"f32.floor", WASM_OP_UNARY, WASM_OPCODE_F32_FLOOR, WASM_TYPE_F32},
#line 117 "src/wasm-keywords.gperf"
      {"i32.store8", WASM_OP_STORE, WASM_OPCODE_I32_STORE_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I8, 4},
#line 65 "src/wasm-keywords.gperf"
      {"f64.le", WASM_OP_COMPARE, WASM_OPCODE_F64_LE, WASM_TYPE_F64},
#line 85 "src/wasm-keywords.gperf"
      {"i32.and", WASM_OP_BINARY, WASM_OPCODE_I32_AND, WASM_TYPE_I32},
#line 138 "src/wasm-keywords.gperf"
      {"i64.ge_u", WASM_OP_COMPARE, WASM_OPCODE_I64_UGE, WASM_TYPE_I64},
#line 192 "src/wasm-keywords.gperf"
      {"type", WASM_OP_TYPE},
      {""},
#line 36 "src/wasm-keywords.gperf"
      {"f32.le", WASM_OP_COMPARE, WASM_OPCODE_F32_LE, WASM_TYPE_F32},
#line 72 "src/wasm-keywords.gperf"
      {"f64.neg", WASM_OP_UNARY, WASM_OPCODE_F64_NEG, WASM_TYPE_F64},
#line 93 "src/wasm-keywords.gperf"
      {"i32.ge_u", WASM_OP_COMPARE, WASM_OPCODE_I32_UGE, WASM_TYPE_I32},
#line 184 "src/wasm-keywords.gperf"
      {"page_size", WASM_OP_PAGE_SIZE, WASM_OPCODE_PAGE_SIZE},
      {""},
#line 153 "src/wasm-keywords.gperf"
      {"i64.ne", WASM_OP_COMPARE, WASM_OPCODE_I64_NE, WASM_TYPE_I64},
#line 43 "src/wasm-keywords.gperf"
      {"f32.neg", WASM_OP_UNARY, WASM_OPCODE_F32_NEG, WASM_TYPE_F32},
#line 76 "src/wasm-keywords.gperf"
      {"f64.sqrt", WASM_OP_UNARY, WASM_OPCODE_F64_SQRT, WASM_TYPE_F64},
#line 156 "src/wasm-keywords.gperf"
      {"i64.reinterpret/f64", WASM_OP_CONVERT, WASM_OPCODE_I64_REINTERPRET_F64, WASM_TYPE_I64, WASM_TYPE_F64},
      {""},
#line 106 "src/wasm-keywords.gperf"
      {"i32.ne", WASM_OP_COMPARE, WASM_OPCODE_I32_NE, WASM_TYPE_I32},
#line 166 "src/wasm-keywords.gperf"
      {"i64.sub", WASM_OP_BINARY, WASM_OPCODE_I64_SUB, WASM_TYPE_I64},
#line 46 "src/wasm-keywords.gperf"
      {"f32.sqrt", WASM_OP_UNARY, WASM_OPCODE_F32_SQRT, WASM_TYPE_F32},
      {""}, {""},
#line 73 "src/wasm-keywords.gperf"
      {"f64.ne", WASM_OP_COMPARE, WASM_OPCODE_F64_NE, WASM_TYPE_F64},
#line 119 "src/wasm-keywords.gperf"
      {"i32.sub", WASM_OP_BINARY, WASM_OPCODE_I32_SUB, WASM_TYPE_I32},
#line 151 "src/wasm-keywords.gperf"
      {"i64.lt_u", WASM_OP_COMPARE, WASM_OPCODE_I64_ULT, WASM_TYPE_I64},
#line 75 "src/wasm-keywords.gperf"
      {"f64.reinterpret/i64", WASM_OP_CONVERT, WASM_OPCODE_F64_REINTERPRET_I64, WASM_TYPE_F64, WASM_TYPE_I64},
      {""},
#line 44 "src/wasm-keywords.gperf"
      {"f32.ne", WASM_OP_COMPARE, WASM_OPCODE_F32_NE, WASM_TYPE_F32},
#line 78 "src/wasm-keywords.gperf"
      {"f64.sub", WASM_OP_BINARY, WASM_OPCODE_F64_SUB, WASM_TYPE_F64},
#line 104 "src/wasm-keywords.gperf"
      {"i32.lt_u", WASM_OP_COMPARE, WASM_OPCODE_I32_ULT, WASM_TYPE_I32},
#line 160 "src/wasm-keywords.gperf"
      {"i64.shr_s", WASM_OP_BINARY, WASM_OPCODE_I64_SAR, WASM_TYPE_I64},
#line 176 "src/wasm-keywords.gperf"
      {"label", WASM_OP_LABEL, WASM_OPCODE_INVALID},
#line 83 "src/wasm-keywords.gperf"
      {"global", WASM_OP_GLOBAL},
#line 48 "src/wasm-keywords.gperf"
      {"f32.sub", WASM_OP_BINARY, WASM_OPCODE_F32_SUB, WASM_TYPE_F32},
#line 142 "src/wasm-keywords.gperf"
      {"i64.le_u", WASM_OP_COMPARE, WASM_OPCODE_I64_ULE, WASM_TYPE_I64},
#line 114 "src/wasm-keywords.gperf"
      {"i32.shr_s", WASM_OP_BINARY, WASM_OPCODE_I32_SAR, WASM_TYPE_I32},
      {""},
#line 71 "src/wasm-keywords.gperf"
      {"f64.nearest", WASM_OP_UNARY, WASM_OPCODE_F64_NEAREST, WASM_TYPE_F64},
#line 125 "src/wasm-keywords.gperf"
      {"i32.wrap/i64", WASM_OP_CONVERT, WASM_OPCODE_I32_CONVERT_I64, WASM_TYPE_I32, WASM_TYPE_I64},
#line 97 "src/wasm-keywords.gperf"
      {"i32.le_u", WASM_OP_COMPARE, WASM_OPCODE_I32_ULE, WASM_TYPE_I32},
#line 30 "src/wasm-keywords.gperf"
      {"f32.demote/f64", WASM_OP_CONVERT, WASM_OPCODE_F32_CONVERT_F64, WASM_TYPE_F32, WASM_TYPE_F64},
      {""},
#line 42 "src/wasm-keywords.gperf"
      {"f32.nearest", WASM_OP_UNARY, WASM_OPCODE_F32_NEAREST, WASM_TYPE_F32},
      {""}, {""}, {""}, {""},
#line 154 "src/wasm-keywords.gperf"
      {"i64.or", WASM_OP_BINARY, WASM_OPCODE_I64_OR, WASM_TYPE_I64},
#line 159 "src/wasm-keywords.gperf"
      {"i64.shl", WASM_OP_BINARY, WASM_OPCODE_I64_SHL, WASM_TYPE_I64},
      {""},
#line 133 "src/wasm-keywords.gperf"
      {"i64.div_u", WASM_OP_BINARY, WASM_OPCODE_I64_UDIV, WASM_TYPE_I64},
      {""},
#line 108 "src/wasm-keywords.gperf"
      {"i32.or", WASM_OP_BINARY, WASM_OPCODE_I32_OR, WASM_TYPE_I32},
#line 113 "src/wasm-keywords.gperf"
      {"i32.shl", WASM_OP_BINARY, WASM_OPCODE_I32_SHL, WASM_TYPE_I32},
      {""},
#line 90 "src/wasm-keywords.gperf"
      {"i32.div_u", WASM_OP_BINARY, WASM_OPCODE_I32_UDIV, WASM_TYPE_I32},
      {""},
#line 188 "src/wasm-keywords.gperf"
      {"return", WASM_OP_RETURN, WASM_OPCODE_RETURN},
#line 131 "src/wasm-keywords.gperf"
      {"i64.ctz", WASM_OP_UNARY, WASM_OPCODE_I64_CTZ, WASM_TYPE_I64},
      {""},
#line 12 "src/wasm-keywords.gperf"
      {"assert_invalid", WASM_OP_ASSERT_INVALID},
#line 15 "src/wasm-keywords.gperf"
      {"break", WASM_OP_BREAK, WASM_OPCODE_BREAK},
#line 175 "src/wasm-keywords.gperf"
      {"invoke", WASM_OP_INVOKE},
#line 88 "src/wasm-keywords.gperf"
      {"i32.ctz", WASM_OP_UNARY, WASM_OPCODE_I32_CTZ, WASM_TYPE_I32},
      {""},
#line 158 "src/wasm-keywords.gperf"
      {"i64.rem_u", WASM_OP_BINARY, WASM_OPCODE_I64_UREM, WASM_TYPE_I64},
      {""},
#line 134 "src/wasm-keywords.gperf"
      {"i64.eq", WASM_OP_COMPARE, WASM_OPCODE_I64_EQ, WASM_TYPE_I64},
      {""}, {""},
#line 112 "src/wasm-keywords.gperf"
      {"i32.rem_u", WASM_OP_BINARY, WASM_OPCODE_I32_UREM, WASM_TYPE_I32},
      {""},
#line 91 "src/wasm-keywords.gperf"
      {"i32.eq", WASM_OP_COMPARE, WASM_OPCODE_I32_EQ, WASM_TYPE_I32},
      {""},
#line 149 "src/wasm-keywords.gperf"
      {"i64.load", WASM_OP_LOAD, WASM_OPCODE_I64_LOAD_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I64, 7},
      {""}, {""},
#line 61 "src/wasm-keywords.gperf"
      {"f64.eq", WASM_OP_COMPARE, WASM_OPCODE_F64_EQ, WASM_TYPE_F64},
      {""},
#line 102 "src/wasm-keywords.gperf"
      {"i32.load", WASM_OP_LOAD, WASM_OPCODE_I32_LOAD_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I32, 6},
#line 179 "src/wasm-keywords.gperf"
      {"loop", WASM_OP_LOOP, WASM_OPCODE_LOOP},
      {""},
#line 32 "src/wasm-keywords.gperf"
      {"f32.eq", WASM_OP_COMPARE, WASM_OPCODE_F32_EQ, WASM_TYPE_F32},
#line 129 "src/wasm-keywords.gperf"
      {"i64.clz", WASM_OP_UNARY, WASM_OPCODE_I64_CLZ, WASM_TYPE_I64},
#line 66 "src/wasm-keywords.gperf"
      {"f64.load", WASM_OP_LOAD, WASM_OPCODE_F64_LOAD_I32, WASM_TYPE_F64, WASM_MEM_TYPE_F64, 0},
#line 19 "src/wasm-keywords.gperf"
      {"call", WASM_OP_CALL, WASM_OPCODE_CALL},
#line 14 "src/wasm-keywords.gperf"
      {"block", WASM_OP_BLOCK, WASM_OPCODE_EXPR_BLOCK},
#line 174 "src/wasm-keywords.gperf"
      {"import", WASM_OP_IMPORT},
#line 86 "src/wasm-keywords.gperf"
      {"i32.clz", WASM_OP_UNARY, WASM_OPCODE_I32_CLZ, WASM_TYPE_I32},
#line 37 "src/wasm-keywords.gperf"
      {"f32.load", WASM_OP_LOAD, WASM_OPCODE_F32_LOAD_I32, WASM_TYPE_F32, WASM_MEM_TYPE_F32, 0},
#line 110 "src/wasm-keywords.gperf"
      {"i32.reinterpret/f32", WASM_OP_CONVERT, WASM_OPCODE_I32_REINTERPRET_F32, WASM_TYPE_I32, WASM_TYPE_F32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 107 "src/wasm-keywords.gperf"
      {"i32.not", WASM_OP_UNARY, WASM_OPCODE_I32_NOT, WASM_TYPE_I32},
#line 183 "src/wasm-keywords.gperf"
      {"nop", WASM_OP_NOP, WASM_OPCODE_NOP, WASM_TYPE_VOID},
#line 45 "src/wasm-keywords.gperf"
      {"f32.reinterpret/i32", WASM_OP_CONVERT, WASM_OPCODE_F32_REINTERPRET_I32, WASM_TYPE_F32, WASM_TYPE_I32},
      {""}, {""},
#line 145 "src/wasm-keywords.gperf"
      {"i64.load32_s", WASM_OP_LOAD, WASM_OPCODE_I64_LOAD_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I32, 6},
#line 53 "src/wasm-keywords.gperf"
      {"f64.ceil", WASM_OP_UNARY, WASM_OPCODE_F64_CEIL, WASM_TYPE_F64},
      {""}, {""}, {""},
#line 143 "src/wasm-keywords.gperf"
      {"i64.load16_s", WASM_OP_LOAD, WASM_OPCODE_I64_LOAD_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I16, 5},
#line 23 "src/wasm-keywords.gperf"
      {"f32.ceil", WASM_OP_UNARY, WASM_OPCODE_F32_CEIL, WASM_TYPE_F32},
#line 81 "src/wasm-keywords.gperf"
      {"func", WASM_OP_FUNC},
#line 155 "src/wasm-keywords.gperf"
      {"i64.popcnt", WASM_OP_UNARY, WASM_OPCODE_I64_POPCNT, WASM_TYPE_I64},
#line 20 "src/wasm-keywords.gperf"
      {"export", WASM_OP_EXPORT},
#line 98 "src/wasm-keywords.gperf"
      {"i32.load16_s", WASM_OP_LOAD, WASM_OPCODE_I32_LOAD_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I16, 5},
      {""}, {""},
#line 109 "src/wasm-keywords.gperf"
      {"i32.popcnt", WASM_OP_UNARY, WASM_OPCODE_I32_POPCNT, WASM_TYPE_I32},
#line 147 "src/wasm-keywords.gperf"
      {"i64.load8_s", WASM_OP_LOAD, WASM_OPCODE_I64_LOAD_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I8, 4},
      {""}, {""}, {""}, {""},
#line 100 "src/wasm-keywords.gperf"
      {"i32.load8_s", WASM_OP_LOAD, WASM_OPCODE_I32_LOAD_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I8, 4},
      {""}, {""}, {""},
#line 178 "src/wasm-keywords.gperf"
      {"local", WASM_OP_LOCAL},
      {""}, {""}, {""},
#line 161 "src/wasm-keywords.gperf"
      {"i64.shr_u", WASM_OP_BINARY, WASM_OPCODE_I64_SHR, WASM_TYPE_I64},
#line 169 "src/wasm-keywords.gperf"
      {"i64.trunc_s/f64", WASM_OP_CONVERT, WASM_OPCODE_I64_SCONVERT_F64, WASM_TYPE_I64, WASM_TYPE_F64},
      {""},
#line 69 "src/wasm-keywords.gperf"
      {"f64.min", WASM_OP_BINARY, WASM_OPCODE_F64_MIN, WASM_TYPE_F64},
      {""},
#line 115 "src/wasm-keywords.gperf"
      {"i32.shr_u", WASM_OP_BINARY, WASM_OPCODE_I32_SHR, WASM_TYPE_I32},
#line 122 "src/wasm-keywords.gperf"
      {"i32.trunc_s/f64", WASM_OP_CONVERT, WASM_OPCODE_I32_SCONVERT_F64, WASM_TYPE_I32, WASM_TYPE_F64},
      {""},
#line 40 "src/wasm-keywords.gperf"
      {"f32.min", WASM_OP_BINARY, WASM_OPCODE_F32_MIN, WASM_TYPE_F32},
#line 10 "src/wasm-keywords.gperf"
      {"assert_return", WASM_OP_ASSERT_RETURN},
      {""}, {""},
#line 163 "src/wasm-keywords.gperf"
      {"i64.store32", WASM_OP_STORE, WASM_OPCODE_I64_STORE_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I32, 6},
#line 11 "src/wasm-keywords.gperf"
      {"assert_return_nan", WASM_OP_ASSERT_RETURN_NAN},
      {""},
#line 82 "src/wasm-keywords.gperf"
      {"get_local", WASM_OP_GET_LOCAL, WASM_OPCODE_GET_LOCAL},
      {""}, {""}, {""}, {""},
#line 189 "src/wasm-keywords.gperf"
      {"set_local", WASM_OP_SET_LOCAL, WASM_OPCODE_SET_LOCAL},
      {""}, {""},
#line 190 "src/wasm-keywords.gperf"
      {"store_global", WASM_OP_STORE_GLOBAL, WASM_OPCODE_SET_GLOBAL},
      {""}, {""},
#line 167 "src/wasm-keywords.gperf"
      {"i64.switch", WASM_OP_SWITCH, WASM_OPCODE_INVALID, WASM_TYPE_I64},
      {""},
#line 152 "src/wasm-keywords.gperf"
      {"i64.mul", WASM_OP_BINARY, WASM_OPCODE_I64_MUL, WASM_TYPE_I64},
      {""}, {""},
#line 120 "src/wasm-keywords.gperf"
      {"i32.switch", WASM_OP_SWITCH, WASM_OPCODE_SWITCH, WASM_TYPE_I32},
      {""},
#line 105 "src/wasm-keywords.gperf"
      {"i32.mul", WASM_OP_BINARY, WASM_OPCODE_I32_MUL, WASM_TYPE_I32},
      {""},
#line 130 "src/wasm-keywords.gperf"
      {"i64.const", WASM_OP_CONST, WASM_OPCODE_I64_CONST, WASM_TYPE_I64},
#line 79 "src/wasm-keywords.gperf"
      {"f64.switch", WASM_OP_SWITCH, WASM_OPCODE_INVALID, WASM_TYPE_F64},
      {""},
#line 70 "src/wasm-keywords.gperf"
      {"f64.mul", WASM_OP_BINARY, WASM_OPCODE_F64_MUL, WASM_TYPE_F64},
      {""},
#line 87 "src/wasm-keywords.gperf"
      {"i32.const", WASM_OP_CONST, WASM_OPCODE_I32_CONST, WASM_TYPE_I32},
#line 49 "src/wasm-keywords.gperf"
      {"f32.switch", WASM_OP_SWITCH, WASM_OPCODE_INVALID, WASM_TYPE_F32},
#line 177 "src/wasm-keywords.gperf"
      {"load_global", WASM_OP_LOAD_GLOBAL, WASM_OPCODE_GET_GLOBAL},
#line 41 "src/wasm-keywords.gperf"
      {"f32.mul", WASM_OP_BINARY, WASM_OPCODE_F32_MUL, WASM_TYPE_F32},
      {""},
#line 54 "src/wasm-keywords.gperf"
      {"f64.const", WASM_OP_CONST, WASM_OPCODE_F64_CONST, WASM_TYPE_F64},
      {""}, {""}, {""}, {""},
#line 24 "src/wasm-keywords.gperf"
      {"f32.const", WASM_OP_CONST, WASM_OPCODE_F32_CONST, WASM_TYPE_F32},
#line 171 "src/wasm-keywords.gperf"
      {"i64.trunc_u/f64", WASM_OP_CONVERT, WASM_OPCODE_I64_UCONVERT_F64, WASM_TYPE_I64, WASM_TYPE_F64},
#line 180 "src/wasm-keywords.gperf"
      {"memory_size", WASM_OP_MEMORY_SIZE, WASM_OPCODE_INVALID},
      {""}, {""},
#line 80 "src/wasm-keywords.gperf"
      {"f64.trunc", WASM_OP_UNARY, WASM_OPCODE_F64_TRUNC, WASM_TYPE_F64},
#line 124 "src/wasm-keywords.gperf"
      {"i32.trunc_u/f64", WASM_OP_CONVERT, WASM_OPCODE_I32_UCONVERT_F64, WASM_TYPE_I32, WASM_TYPE_F64},
      {""}, {""}, {""},
#line 50 "src/wasm-keywords.gperf"
      {"f32.trunc", WASM_OP_UNARY, WASM_OPCODE_F32_TRUNC, WASM_TYPE_F32},
      {""},
#line 17 "src/wasm-keywords.gperf"
      {"call_import", WASM_OP_CALL_IMPORT, WASM_OPCODE_CALL},
#line 172 "src/wasm-keywords.gperf"
      {"i64.xor", WASM_OP_BINARY, WASM_OPCODE_I64_XOR, WASM_TYPE_I64},
      {""}, {""}, {""}, {""},
#line 126 "src/wasm-keywords.gperf"
      {"i32.xor", WASM_OP_BINARY, WASM_OPCODE_I32_XOR, WASM_TYPE_I32},
      {""}, {""},
#line 168 "src/wasm-keywords.gperf"
      {"i64.trunc_s/f32", WASM_OP_CONVERT, WASM_OPCODE_I64_SCONVERT_F32, WASM_TYPE_I64, WASM_TYPE_F32},
#line 181 "src/wasm-keywords.gperf"
      {"memory", WASM_OP_MEMORY},
#line 146 "src/wasm-keywords.gperf"
      {"i64.load32_u", WASM_OP_LOAD, WASM_OPCODE_I64_LOAD_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I32, 2},
      {""}, {""},
#line 121 "src/wasm-keywords.gperf"
      {"i32.trunc_s/f32", WASM_OP_CONVERT, WASM_OPCODE_I32_SCONVERT_F32, WASM_TYPE_I32, WASM_TYPE_F32},
      {""},
#line 144 "src/wasm-keywords.gperf"
      {"i64.load16_u", WASM_OP_LOAD, WASM_OPCODE_I64_LOAD_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I16, 1},
      {""}, {""}, {""}, {""},
#line 99 "src/wasm-keywords.gperf"
      {"i32.load16_u", WASM_OP_LOAD, WASM_OPCODE_I32_LOAD_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I16, 1},
      {""}, {""}, {""},
#line 148 "src/wasm-keywords.gperf"
      {"i64.load8_u", WASM_OP_LOAD, WASM_OPCODE_I64_LOAD_I32, WASM_TYPE_I64, WASM_MEM_TYPE_I8, 0},
      {""},
#line 18 "src/wasm-keywords.gperf"
      {"call_indirect", WASM_OP_CALL_INDIRECT, WASM_OPCODE_CALL_INDIRECT},
      {""},
#line 74 "src/wasm-keywords.gperf"
      {"f64.promote/f32", WASM_OP_CONVERT, WASM_OPCODE_F64_CONVERT_F32, WASM_TYPE_F64, WASM_TYPE_F32},
#line 101 "src/wasm-keywords.gperf"
      {"i32.load8_u", WASM_OP_LOAD, WASM_OPCODE_I32_LOAD_I32, WASM_TYPE_I32, WASM_MEM_TYPE_I8, 0},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 56 "src/wasm-keywords.gperf"
      {"f64.convert_s/i64", WASM_OP_CONVERT, WASM_OPCODE_F64_SCONVERT_I64, WASM_TYPE_F64, WASM_TYPE_I64},
      {""}, {""},
#line 185 "src/wasm-keywords.gperf"
      {"param", WASM_OP_PARAM},
#line 182 "src/wasm-keywords.gperf"
      {"module", WASM_OP_MODULE},
#line 26 "src/wasm-keywords.gperf"
      {"f32.convert_s/i64", WASM_OP_CONVERT, WASM_OPCODE_F32_SCONVERT_I64, WASM_TYPE_F32, WASM_TYPE_I64},
      {""}, {""}, {""}, {""},
#line 68 "src/wasm-keywords.gperf"
      {"f64.max", WASM_OP_BINARY, WASM_OPCODE_F64_MAX, WASM_TYPE_F64},
      {""}, {""}, {""}, {""},
#line 39 "src/wasm-keywords.gperf"
      {"f32.max", WASM_OP_BINARY, WASM_OPCODE_F32_MAX, WASM_TYPE_F32},
      {""}, {""},
#line 170 "src/wasm-keywords.gperf"
      {"i64.trunc_u/f32", WASM_OP_CONVERT, WASM_OPCODE_I64_UCONVERT_F32, WASM_TYPE_I64, WASM_TYPE_F32},
      {""}, {""}, {""}, {""},
#line 123 "src/wasm-keywords.gperf"
      {"i32.trunc_u/f32", WASM_OP_CONVERT, WASM_OPCODE_I32_UCONVERT_F32, WASM_TYPE_I32, WASM_TYPE_F32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 59 "src/wasm-keywords.gperf"
      {"f64.copysign", WASM_OP_BINARY, WASM_OPCODE_F64_COPYSIGN, WASM_TYPE_F64},
      {""}, {""}, {""}, {""},
#line 29 "src/wasm-keywords.gperf"
      {"f32.copysign", WASM_OP_BINARY, WASM_OPCODE_F32_COPYSIGN, WASM_TYPE_F32},
      {""}, {""}, {""}, {""},
#line 58 "src/wasm-keywords.gperf"
      {"f64.convert_u/i64", WASM_OP_CONVERT, WASM_OPCODE_F64_UCONVERT_I64, WASM_TYPE_F64, WASM_TYPE_I64},
      {""}, {""}, {""}, {""},
#line 28 "src/wasm-keywords.gperf"
      {"f32.convert_u/i64", WASM_OP_CONVERT, WASM_OPCODE_F32_UCONVERT_I64, WASM_TYPE_F32, WASM_TYPE_I64},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 135 "src/wasm-keywords.gperf"
      {"i64.extend_s/i32", WASM_OP_CONVERT, WASM_OPCODE_I64_SCONVERT_I32, WASM_TYPE_I64, WASM_TYPE_I32},
      {""}, {""}, {""}, {""}, {""},
#line 55 "src/wasm-keywords.gperf"
      {"f64.convert_s/i32", WASM_OP_CONVERT, WASM_OPCODE_F64_SCONVERT_I32, WASM_TYPE_F64, WASM_TYPE_I32},
      {""}, {""}, {""}, {""},
#line 25 "src/wasm-keywords.gperf"
      {"f32.convert_s/i32", WASM_OP_CONVERT, WASM_OPCODE_F32_SCONVERT_I32, WASM_TYPE_F32, WASM_TYPE_I32},
      {""}, {""}, {""},
#line 186 "src/wasm-keywords.gperf"
      {"grow_memory", WASM_OP_GROW_MEMORY, WASM_OPCODE_RESIZE_MEMORY_I32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 136 "src/wasm-keywords.gperf"
      {"i64.extend_u/i32", WASM_OP_CONVERT, WASM_OPCODE_I64_UCONVERT_I32, WASM_TYPE_I64, WASM_TYPE_I32},
      {""}, {""}, {""}, {""}, {""},
#line 57 "src/wasm-keywords.gperf"
      {"f64.convert_u/i32", WASM_OP_CONVERT, WASM_OPCODE_F64_UCONVERT_I32, WASM_TYPE_F64, WASM_TYPE_I32},
      {""}, {""}, {""}, {""},
#line 27 "src/wasm-keywords.gperf"
      {"f32.convert_u/i32", WASM_OP_CONVERT, WASM_OPCODE_F32_UCONVERT_I32, WASM_TYPE_F32, WASM_TYPE_I32}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}

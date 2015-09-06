/* C code produced by gperf version 3.0.4 */
/* Command-line: gperf --compare-strncmp --readonly-tables --struct-type --output-file hash.h hash.txt  */
/* Computed positions: -k'1-2,5-6,10-13,$' */

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

#line 1 "hash.txt"
struct OpInfo {
  const char* name;
  int op_type;
  int opcode;
  int type;
  int type2;
  int access;
};

#define TOTAL_KEYWORDS 188
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 9
#define MAX_HASH_VALUE 474
/* maximum key range = 466, duplicates = 0 */

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
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475,   0, 475, 205,
       20,   5,  75, 475,   0, 475, 155, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475,   0, 475,  90,  69, 115,
        0, 105,  10,  90,   0,   0, 475,   5,  10,  19,
       25,   0,  55,  15,  35,  10,   0,   0,  25, 140,
      135, 475, 165, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475, 475, 475, 475, 475,
      475, 475, 475, 475, 475, 475
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
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
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
#line 137 "hash.txt"
      {"i64.div_u", OP_BINARY, OPCODE_I64_UDIV, TYPE_I64},
      {""}, {""}, {""}, {""},
#line 84 "hash.txt"
      {"i32.div_u", OP_BINARY, OPCODE_I32_UDIV, TYPE_I32},
      {""}, {""}, {""},
#line 160 "hash.txt"
      {"i64.lt_u", OP_COMPARE, OPCODE_I64_ULT, TYPE_I64},
#line 136 "hash.txt"
      {"i64.div_s", OP_BINARY, OPCODE_I64_SDIV, TYPE_I64},
      {""}, {""},
#line 190 "hash.txt"
      {"if", OP_IF, OPCODE_IF},
#line 102 "hash.txt"
      {"i32.lt_u", OP_COMPARE, OPCODE_I32_ULT, TYPE_I32},
#line 83 "hash.txt"
      {"i32.div_s", OP_BINARY, OPCODE_I32_SDIV, TYPE_I32},
#line 179 "hash.txt"
      {"i64.store_u/i16", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I16, 1},
#line 62 "hash.txt"
      {"f64.lt", OP_COMPARE, OPCODE_F64_LT, TYPE_F64},
#line 169 "hash.txt"
      {"i64.shl", OP_BINARY, OPCODE_I64_SHL, TYPE_I64},
#line 159 "hash.txt"
      {"i64.lt_s", OP_COMPARE, OPCODE_I64_SLT, TYPE_I64},
      {""},
#line 120 "hash.txt"
      {"i32.store_u/i16", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I16, 1},
#line 32 "hash.txt"
      {"f32.lt", OP_COMPARE, OPCODE_F32_LT, TYPE_F32},
#line 112 "hash.txt"
      {"i32.shl", OP_BINARY, OPCODE_I32_SHL, TYPE_I32},
#line 101 "hash.txt"
      {"i32.lt_s", OP_COMPARE, OPCODE_I32_SLT, TYPE_I32},
      {""},
#line 175 "hash.txt"
      {"i64.store_s/i16", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I16, 5},
#line 161 "hash.txt"
      {"i64.mul", OP_BINARY, OPCODE_I64_MUL, TYPE_I64},
#line 105 "hash.txt"
      {"i32.not", OP_UNARY, OPCODE_I32_NOT, TYPE_I32},
      {""}, {""},
#line 117 "hash.txt"
      {"i32.store_s/i16", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I16, 5},
#line 103 "hash.txt"
      {"i32.mul", OP_BINARY, OPCODE_I32_MUL, TYPE_I32},
#line 54 "hash.txt"
      {"f64.div", OP_BINARY, OPCODE_F64_DIV, TYPE_F64},
#line 71 "hash.txt"
      {"f64.sqrt", OP_UNARY, OPCODE_F64_SQRT, TYPE_F64},
      {""},
#line 180 "hash.txt"
      {"i64.store_u/i32", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I32, 2},
#line 65 "hash.txt"
      {"f64.mul", OP_BINARY, OPCODE_F64_MUL, TYPE_F64},
#line 25 "hash.txt"
      {"f32.div", OP_BINARY, OPCODE_F32_DIV, TYPE_F32},
#line 40 "hash.txt"
      {"f32.sqrt", OP_UNARY, OPCODE_F32_SQRT, TYPE_F32},
#line 156 "hash.txt"
      {"i64.load_u/i32", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I32, 2},
#line 121 "hash.txt"
      {"i32.store_u/i32", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I32, 2},
#line 35 "hash.txt"
      {"f32.mul", OP_BINARY, OPCODE_F32_MUL, TYPE_F32},
#line 170 "hash.txt"
      {"i64.shr", OP_BINARY, OPCODE_I64_SHR, TYPE_I64},
      {""},
#line 99 "hash.txt"
      {"i32.load_u/i32", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I32, 2},
#line 176 "hash.txt"
      {"i64.store_s/i32", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I32, 6},
      {""},
#line 113 "hash.txt"
      {"i32.shr", OP_BINARY, OPCODE_I32_SHR, TYPE_I32},
      {""},
#line 152 "hash.txt"
      {"i64.load_s/i32", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I32, 6},
#line 118 "hash.txt"
      {"i32.store_s/i32", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I32, 6},
#line 64 "hash.txt"
      {"f64.min", OP_BINARY, OPCODE_F64_MIN, TYPE_F64},
      {""}, {""},
#line 96 "hash.txt"
      {"i32.load_s/i32", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I32, 6},
#line 164 "hash.txt"
      {"i64.popcnt", OP_UNARY, OPCODE_I64_POPCNT, TYPE_I64},
#line 34 "hash.txt"
      {"f32.min", OP_BINARY, OPCODE_F32_MIN, TYPE_F32},
#line 148 "hash.txt"
      {"i64.load/i32", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I32, 6},
#line 172 "hash.txt"
      {"i64.store/i32", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I32, 6},
#line 193 "hash.txt"
      {"loop", OP_LOOP, OPCODE_LOOP},
#line 107 "hash.txt"
      {"i32.popcnt", OP_UNARY, OPCODE_I32_POPCNT, TYPE_I32},
      {""},
#line 93 "hash.txt"
      {"i32.load/i32", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I32, 6},
#line 115 "hash.txt"
      {"i32.store/i32", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I32, 6},
#line 56 "hash.txt"
      {"f64.floor", OP_UNARY, OPCODE_F64_FLOOR, TYPE_F64},
      {""},
#line 163 "hash.txt"
      {"i64.or", OP_BINARY, OPCODE_I64_OR, TYPE_I64},
      {""}, {""},
#line 27 "hash.txt"
      {"f32.floor", OP_UNARY, OPCODE_F32_FLOOR, TYPE_F32},
#line 187 "hash.txt"
      {"i64.trunc_u/f32", OP_CONVERT, OPCODE_I64_UCONVERT_F32, TYPE_I64, TYPE_F32},
#line 106 "hash.txt"
      {"i32.or", OP_BINARY, OPCODE_I32_OR, TYPE_I32},
      {""},
#line 194 "hash.txt"
      {"nop", OP_NOP, OPCODE_NOP, TYPE_VOID},
      {""},
#line 127 "hash.txt"
      {"i32.trunc_u/f32", OP_CONVERT, OPCODE_I32_UCONVERT_F32, TYPE_I32, TYPE_F32},
#line 183 "hash.txt"
      {"i64.sub", OP_BINARY, OPCODE_I64_SUB, TYPE_I64},
#line 60 "hash.txt"
      {"f64.load/f32", OP_LOAD, OPCODE_INVALID, TYPE_F64, MEM_TYPE_F32, 0},
#line 72 "hash.txt"
      {"f64.store/f32", OP_STORE, OPCODE_INVALID, TYPE_F64, MEM_TYPE_F32, 0},
      {""},
#line 185 "hash.txt"
      {"i64.trunc_s/f32", OP_CONVERT, OPCODE_I64_SCONVERT_F32, TYPE_I64, TYPE_F32},
#line 123 "hash.txt"
      {"i32.sub", OP_BINARY, OPCODE_I32_SUB, TYPE_I32},
#line 31 "hash.txt"
      {"f32.load/f32", OP_LOAD, OPCODE_F32_LOAD_I32, TYPE_F32, MEM_TYPE_F32, 0},
#line 41 "hash.txt"
      {"f32.store/f32", OP_STORE, OPCODE_F32_STORE_I32, TYPE_F32, MEM_TYPE_F32, 0},
#line 10 "hash.txt"
      {"block", OP_BLOCK, OPCODE_BLOCK},
#line 125 "hash.txt"
      {"i32.trunc_s/f32", OP_CONVERT, OPCODE_I32_SCONVERT_F32, TYPE_I32, TYPE_F32},
#line 74 "hash.txt"
      {"f64.sub", OP_BINARY, OPCODE_F64_SUB, TYPE_F64},
#line 131 "hash.txt"
      {"i64.add", OP_BINARY, OPCODE_I64_ADD, TYPE_I64},
#line 144 "hash.txt"
      {"i64.gt_u", OP_COMPARE, OPCODE_I64_UGT, TYPE_I64},
#line 157 "hash.txt"
      {"i64.load_u/i64", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I64, 3},
#line 181 "hash.txt"
      {"i64.store_u/i64", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I64, 3},
#line 42 "hash.txt"
      {"f32.sub", OP_BINARY, OPCODE_F32_SUB, TYPE_F32},
#line 78 "hash.txt"
      {"i32.add", OP_BINARY, OPCODE_I32_ADD, TYPE_I32},
#line 89 "hash.txt"
      {"i32.gt_u", OP_COMPARE, OPCODE_I32_UGT, TYPE_I32},
      {""}, {""},
#line 58 "hash.txt"
      {"f64.gt", OP_COMPARE, OPCODE_F64_GT, TYPE_F64},
#line 46 "hash.txt"
      {"f64.add", OP_BINARY, OPCODE_F64_ADD, TYPE_F64},
#line 143 "hash.txt"
      {"i64.gt_s", OP_COMPARE, OPCODE_I64_SGT, TYPE_I64},
#line 153 "hash.txt"
      {"i64.load_s/i64", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I64, 7},
#line 177 "hash.txt"
      {"i64.store_s/i64", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I64, 7},
#line 29 "hash.txt"
      {"f32.gt", OP_COMPARE, OPCODE_F32_GT, TYPE_F32},
#line 16 "hash.txt"
      {"f32.add", OP_BINARY, OPCODE_F32_ADD, TYPE_F32},
#line 88 "hash.txt"
      {"i32.gt_s", OP_COMPARE, OPCODE_I32_SGT, TYPE_I32},
      {""}, {""}, {""}, {""}, {""},
#line 11 "hash.txt"
      {"break", OP_BREAK, OPCODE_BREAK},
      {""}, {""},
#line 132 "hash.txt"
      {"i64.and", OP_BINARY, OPCODE_I64_AND, TYPE_I64},
#line 146 "hash.txt"
      {"i64.le_u", OP_COMPARE, OPCODE_I64_ULE, TYPE_I64},
#line 134 "hash.txt"
      {"i64.const", OP_CONST, OPCODE_I64_CONST, TYPE_I64},
#line 191 "hash.txt"
      {"label", OP_LABEL, OPCODE_INVALID},
      {""},
#line 79 "hash.txt"
      {"i32.and", OP_BINARY, OPCODE_I32_AND, TYPE_I32},
#line 91 "hash.txt"
      {"i32.le_u", OP_COMPARE, OPCODE_I32_ULE, TYPE_I32},
#line 81 "hash.txt"
      {"i32.const", OP_CONST, OPCODE_I32_CONST, TYPE_I32},
      {""}, {""}, {""},
#line 145 "hash.txt"
      {"i64.le_s", OP_COMPARE, OPCODE_I64_SLE, TYPE_I64},
#line 48 "hash.txt"
      {"f64.const", OP_CONST, OPCODE_F64_CONST, TYPE_F64},
#line 188 "hash.txt"
      {"i64.trunc_u/f64", OP_CONVERT, OPCODE_I64_UCONVERT_F64, TYPE_I64, TYPE_F64},
      {""}, {""},
#line 90 "hash.txt"
      {"i32.le_s", OP_COMPARE, OPCODE_I32_SLE, TYPE_I32},
#line 18 "hash.txt"
      {"f32.const", OP_CONST, OPCODE_F32_CONST, TYPE_F32},
#line 128 "hash.txt"
      {"i32.trunc_u/f64", OP_CONVERT, OPCODE_I32_UCONVERT_F64, TYPE_I32, TYPE_F64},
#line 138 "hash.txt"
      {"i64.eq", OP_COMPARE, OPCODE_I64_EQ, TYPE_I64},
#line 168 "hash.txt"
      {"i64.sar", OP_BINARY, OPCODE_I64_SAR, TYPE_I64},
      {""},
#line 196 "hash.txt"
      {"set_local", OP_SET_LOCAL, OPCODE_SET_LOCAL},
#line 186 "hash.txt"
      {"i64.trunc_s/f64", OP_CONVERT, OPCODE_I64_SCONVERT_F64, TYPE_I64, TYPE_F64},
#line 85 "hash.txt"
      {"i32.eq", OP_COMPARE, OPCODE_I32_EQ, TYPE_I32},
#line 111 "hash.txt"
      {"i32.sar", OP_BINARY, OPCODE_I32_SAR, TYPE_I32},
#line 14 "hash.txt"
      {"destruct", OP_DESTRUCT, OPCODE_INVALID},
#line 167 "hash.txt"
      {"i64.rem_u", OP_BINARY, OPCODE_I64_UREM, TYPE_I64},
#line 126 "hash.txt"
      {"i32.trunc_s/f64", OP_CONVERT, OPCODE_I32_SCONVERT_F64, TYPE_I32, TYPE_F64},
#line 55 "hash.txt"
      {"f64.eq", OP_COMPARE, OPCODE_F64_EQ, TYPE_F64},
#line 162 "hash.txt"
      {"i64.neq", OP_COMPARE, OPCODE_I64_NE, TYPE_I64},
      {""},
#line 110 "hash.txt"
      {"i32.rem_u", OP_BINARY, OPCODE_I32_UREM, TYPE_I32},
      {""},
#line 26 "hash.txt"
      {"f32.eq", OP_COMPARE, OPCODE_F32_EQ, TYPE_F32},
#line 104 "hash.txt"
      {"i32.neq", OP_COMPARE, OPCODE_I32_NE, TYPE_I32},
      {""},
#line 166 "hash.txt"
      {"i64.rem_s", OP_BINARY, OPCODE_I64_SREM, TYPE_I64},
#line 184 "hash.txt"
      {"i64.switch", OP_SWITCH, OPCODE_INVALID, TYPE_I64},
#line 66 "hash.txt"
      {"f64.nearest", OP_UNARY, OPCODE_F64_NEAREST, TYPE_F64},
#line 68 "hash.txt"
      {"f64.neq", OP_COMPARE, OPCODE_F64_NE, TYPE_F64},
      {""},
#line 109 "hash.txt"
      {"i32.rem_s", OP_BINARY, OPCODE_I32_SREM, TYPE_I32},
#line 124 "hash.txt"
      {"i32.switch", OP_SWITCH, OPCODE_SWITCH, TYPE_I32},
#line 36 "hash.txt"
      {"f32.nearest", OP_UNARY, OPCODE_F32_NEAREST, TYPE_F32},
#line 38 "hash.txt"
      {"f32.neq", OP_COMPARE, OPCODE_F32_NE, TYPE_F32},
      {""},
#line 76 "hash.txt"
      {"f64.trunc", OP_UNARY, OPCODE_F64_TRUNC, TYPE_F64},
#line 75 "hash.txt"
      {"f64.switch", OP_SWITCH, OPCODE_INVALID, TYPE_F64},
      {""},
#line 149 "hash.txt"
      {"i64.load/i64", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I64, 7},
#line 173 "hash.txt"
      {"i64.store/i64", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I64, 7},
#line 44 "hash.txt"
      {"f32.trunc", OP_UNARY, OPCODE_F32_TRUNC, TYPE_F32},
#line 43 "hash.txt"
      {"f32.switch", OP_SWITCH, OPCODE_INVALID, TYPE_F32},
      {""},
#line 189 "hash.txt"
      {"i64.xor", OP_BINARY, OPCODE_I64_XOR, TYPE_I64},
      {""},
#line 182 "hash.txt"
      {"i64.store_u/i8", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I8, 0},
      {""}, {""},
#line 130 "hash.txt"
      {"i32.xor", OP_BINARY, OPCODE_I32_XOR, TYPE_I32},
      {""},
#line 122 "hash.txt"
      {"i32.store_u/i8", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I8, 0},
      {""},
#line 45 "hash.txt"
      {"f64.abs", OP_UNARY, OPCODE_F64_ABS, TYPE_F64},
      {""}, {""},
#line 178 "hash.txt"
      {"i64.store_s/i8", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I8, 4},
      {""},
#line 15 "hash.txt"
      {"f32.abs", OP_UNARY, OPCODE_F32_ABS, TYPE_F32},
#line 61 "hash.txt"
      {"f64.load/f64", OP_LOAD, OPCODE_F64_LOAD_I32, TYPE_F64, MEM_TYPE_F64, 0},
#line 73 "hash.txt"
      {"f64.store/f64", OP_STORE, OPCODE_F64_STORE_I32, TYPE_F64, MEM_TYPE_F64, 0},
#line 119 "hash.txt"
      {"i32.store_s/i8", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I8, 4},
      {""}, {""},
#line 51 "hash.txt"
      {"f64.convert_u/i32", OP_CONVERT, OPCODE_F64_UCONVERT_I32, TYPE_F64, TYPE_I32},
      {""}, {""}, {""}, {""},
#line 21 "hash.txt"
      {"f32.convert_u/i32", OP_CONVERT, OPCODE_F32_UCONVERT_I32, TYPE_F32, TYPE_I32},
#line 142 "hash.txt"
      {"i64.ge_u", OP_COMPARE, OPCODE_I64_UGE, TYPE_I64},
      {""}, {""}, {""},
#line 49 "hash.txt"
      {"f64.convert_s/i32", OP_CONVERT, OPCODE_F64_SCONVERT_I32, TYPE_F64, TYPE_I32},
#line 87 "hash.txt"
      {"i32.ge_u", OP_COMPARE, OPCODE_I32_UGE, TYPE_I32},
      {""}, {""}, {""},
#line 19 "hash.txt"
      {"f32.convert_s/i32", OP_CONVERT, OPCODE_F32_SCONVERT_I32, TYPE_F32, TYPE_I32},
#line 141 "hash.txt"
      {"i64.ge_s", OP_COMPARE, OPCODE_I64_SGE, TYPE_I64},
      {""}, {""}, {""}, {""},
#line 86 "hash.txt"
      {"i32.ge_s", OP_COMPARE, OPCODE_I32_SGE, TYPE_I32},
#line 12 "hash.txt"
      {"call", OP_CALL, OPCODE_CALL},
      {""},
#line 192 "hash.txt"
      {"load_global", OP_LOAD_GLOBAL, OPCODE_GET_GLOBAL},
      {""}, {""},
#line 77 "hash.txt"
      {"get_local", OP_GET_LOCAL, OPCODE_GET_LOCAL},
      {""}, {""},
#line 147 "hash.txt"
      {"i64.load/i16", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I16, 5},
#line 171 "hash.txt"
      {"i64.store/i16", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I16, 5},
#line 155 "hash.txt"
      {"i64.load_u/i16", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I16, 1},
      {""},
#line 195 "hash.txt"
      {"return", OP_RETURN, OPCODE_RETURN},
#line 92 "hash.txt"
      {"i32.load/i16", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I16, 5},
#line 114 "hash.txt"
      {"i32.store/i16", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I16, 5},
#line 98 "hash.txt"
      {"i32.load_u/i16", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I16, 1},
      {""},
#line 59 "hash.txt"
      {"f64.le", OP_COMPARE, OPCODE_F64_LE, TYPE_F64},
#line 67 "hash.txt"
      {"f64.neg", OP_UNARY, OPCODE_F64_NEG, TYPE_F64},
      {""},
#line 151 "hash.txt"
      {"i64.load_s/i16", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I16, 5},
      {""},
#line 30 "hash.txt"
      {"f32.le", OP_COMPARE, OPCODE_F32_LE, TYPE_F32},
#line 37 "hash.txt"
      {"f32.neg", OP_UNARY, OPCODE_F32_NEG, TYPE_F32},
      {""},
#line 95 "hash.txt"
      {"i32.load_s/i16", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I16, 5},
      {""}, {""}, {""},
#line 47 "hash.txt"
      {"f64.ceil", OP_UNARY, OPCODE_F64_CEIL, TYPE_F64},
      {""},
#line 69 "hash.txt"
      {"f64.promote/f32", OP_CONVERT, OPCODE_F64_CONVERT_F32, TYPE_F64, TYPE_F32},
      {""},
#line 52 "hash.txt"
      {"f64.convert_u/i64", OP_CONVERT, OPCODE_F64_UCONVERT_I64, TYPE_F64, TYPE_I64},
#line 17 "hash.txt"
      {"f32.ceil", OP_UNARY, OPCODE_F32_CEIL, TYPE_F32},
      {""}, {""}, {""},
#line 22 "hash.txt"
      {"f32.convert_u/i64", OP_CONVERT, OPCODE_F32_UCONVERT_I64, TYPE_F32, TYPE_I64},
      {""}, {""}, {""},
#line 63 "hash.txt"
      {"f64.max", OP_BINARY, OPCODE_F64_MAX, TYPE_F64},
#line 50 "hash.txt"
      {"f64.convert_s/i64", OP_CONVERT, OPCODE_F64_SCONVERT_I64, TYPE_F64, TYPE_I64},
      {""}, {""}, {""},
#line 33 "hash.txt"
      {"f32.max", OP_BINARY, OPCODE_F32_MAX, TYPE_F32},
#line 20 "hash.txt"
      {"f32.convert_s/i64", OP_CONVERT, OPCODE_F32_SCONVERT_I64, TYPE_F32, TYPE_I64},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 140 "hash.txt"
      {"i64.extend_u/i32", OP_CONVERT, OPCODE_I64_UCONVERT_I32, TYPE_I64, TYPE_I32},
#line 53 "hash.txt"
      {"f64.copysign", OP_BINARY, OPCODE_F64_COPYSIGN, TYPE_F64},
      {""}, {""}, {""}, {""},
#line 23 "hash.txt"
      {"f32.copysign", OP_BINARY, OPCODE_F32_COPYSIGN, TYPE_F32},
      {""}, {""}, {""},
#line 139 "hash.txt"
      {"i64.extend_s/i32", OP_CONVERT, OPCODE_I64_SCONVERT_I32, TYPE_I64, TYPE_I32},
#line 135 "hash.txt"
      {"i64.ctz", OP_UNARY, OPCODE_I64_CTZ, TYPE_I64},
      {""}, {""}, {""}, {""},
#line 82 "hash.txt"
      {"i32.ctz", OP_UNARY, OPCODE_I32_CTZ, TYPE_I32},
      {""}, {""}, {""}, {""},
#line 133 "hash.txt"
      {"i64.clz", OP_UNARY, OPCODE_I64_CLZ, TYPE_I64},
      {""}, {""}, {""}, {""},
#line 80 "hash.txt"
      {"i32.clz", OP_UNARY, OPCODE_I32_CLZ, TYPE_I32},
      {""}, {""}, {""},
#line 197 "hash.txt"
      {"store_global", OP_STORE_GLOBAL, OPCODE_SET_GLOBAL},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 57 "hash.txt"
      {"f64.ge", OP_COMPARE, OPCODE_F64_GE, TYPE_F64},
      {""}, {""}, {""}, {""},
#line 28 "hash.txt"
      {"f32.ge", OP_COMPARE, OPCODE_F32_GE, TYPE_F32},
      {""}, {""},
#line 24 "hash.txt"
      {"f32.demote/f64", OP_CONVERT, OPCODE_F32_CONVERT_F64, TYPE_F32, TYPE_F64},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 150 "hash.txt"
      {"i64.load/i8", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I8, 4},
#line 174 "hash.txt"
      {"i64.store/i8", OP_STORE, OPCODE_I64_STORE_I32, TYPE_I64, MEM_TYPE_I8, 4},
#line 158 "hash.txt"
      {"i64.load_u/i8", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I8, 0},
      {""}, {""},
#line 94 "hash.txt"
      {"i32.load/i8", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I8, 4},
#line 116 "hash.txt"
      {"i32.store/i8", OP_STORE, OPCODE_I32_STORE_I32, TYPE_I32, MEM_TYPE_I8, 4},
#line 100 "hash.txt"
      {"i32.load_u/i8", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I8, 0},
      {""}, {""}, {""},
#line 129 "hash.txt"
      {"i32.wrap/i64", OP_CONVERT, OPCODE_I32_CONVERT_I64, TYPE_I32, TYPE_I64},
#line 154 "hash.txt"
      {"i64.load_s/i8", OP_LOAD, OPCODE_I64_LOAD_I32, TYPE_I64, MEM_TYPE_I8, 4},
      {""}, {""}, {""}, {""},
#line 97 "hash.txt"
      {"i32.load_s/i8", OP_LOAD, OPCODE_I32_LOAD_I32, TYPE_I32, MEM_TYPE_I8, 4},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 108 "hash.txt"
      {"i32.reinterpret/f32", OP_CONVERT, OPCODE_INVALID, TYPE_I32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 39 "hash.txt"
      {"f32.reinterpret/i32", OP_CONVERT, OPCODE_F32_REINTERPRET_I32, TYPE_F32, TYPE_I32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 165 "hash.txt"
      {"i64.reinterpret/f64", OP_CONVERT, OPCODE_INVALID, TYPE_I64},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 13 "hash.txt"
      {"call_indirect", OP_CALL_INDIRECT, OPCODE_CALL_INDIRECT},
#line 70 "hash.txt"
      {"f64.reinterpret/i64", OP_CONVERT, OPCODE_F64_REINTERPRET_I64, TYPE_F64, TYPE_I64}
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

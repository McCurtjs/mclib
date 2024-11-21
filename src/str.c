/*******************************************************************************
* MIT License
*
* Copyright (c) 2024 Curtis McCoy
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "str.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "utility.h"

#undef SRCV
#define SRCV
typedef struct {
  // public (read-only)
  union {
    StringRange range;
    _STR_RANGE_DEF(,);
  };

  // private
  char head;
} String_Internal;

static char const str_chr_literal_empty = '\0';
static struct _Str_Base str_constants[] = {
  { .begin = &str_chr_literal_empty, .size = 0 },
  { .begin = &str_chr_literal_empty, .size = 0 },
  { .begin = "true", .size = 4 },
  { .begin = "false", .size = 5 },
};
static String const str_constants_end = &str_constants[0] + ARRAY_COUNT(str_constants);

const String str_empty  = &str_constants[0];
const String str_va_end = &str_constants[1];
const String str_true   = &str_constants[2];
const String str_false  = &str_constants[3];

static String_Internal* str_new_internal(index_s length) {
  if (length == 0) return NULL; // prompt callers to return empty string
  // Include an extra byte for the null terminator
  String_Internal* ret = malloc(sizeof(StringRange) + length + 1);
  assert(ret);
  ret->begin = &ret->head;
  ret->size = length;
  return ret;
}

#ifdef __GNUC__
// This warning detects that the begin pointer actually pionts to the head value
//    which is just a char, and is too small to write into an offset of. But
//    we're doing that on purpose by allocating extra memory for the string,
//    so ignore the warning.
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstringop-overflow="
#endif
inline static String str_terminate(String_Internal* str) {
  *(str->begin + str->length) = '\0';
  return (String)str;
}
#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif

inline static bool str_is_literal(String str) {
  return (str >= &str_constants[0] && str < str_constants_end);
}

////////////////////////////////////////////////////////////////////////////////
// Direct string str_ functions
////////////////////////////////////////////////////////////////////////////////

StringRange str_range(const char* c_str) {
  return (StringRange) {
    .begin = c_str,
    .length = strlen(c_str),
  };
}

StringRange str_range_s(const char* c_str, index_s length) {
  return (StringRange) {
    .begin = c_str,
    .length = length,
  };
}

String str_new(const char* c_str) {
  if (!c_str) return str_empty;
  index_s length = strlen(c_str);
  String_Internal* ret = str_new_internal(length);
  if (!ret) return str_empty;
  memcpy(&ret->head, c_str, length);
  return str_terminate(ret);
}

String str_new_s(const char* c_str, index_s length) {
  if (!c_str) return str_empty;
  String_Internal* ret = str_new_internal(length);
  if (!ret) return str_empty;
  memcpy(&ret->head, c_str, length);
  return str_terminate(ret);
}

String str_from_bool(bool b) {
  return b == FALSE ? str_false : str_true;
}

String str_from_int(int i) {
  return str_new(itos(i));
}

String str_from_float(float f) {
  return str_new(ftos(f));
}

void str_delete(String* str) {
  if (!str || !*str) return;
  if (!str_is_literal(*str)) free(*str);
  *str = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Macro-overloaded str_ via istr_ functions
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
void istr_write(StringRange str) {
  uint length = (uint)str.size;
  printf("%.*s\n", length, str.begin);
  fflush(stdout); // SDL Window blocks console output for some reason
}

String istr_copy(StringRange str) {
  String_Internal* ret = str_new_internal(str.size);
  if (!ret) return str_empty;
  memcpy(ret->begin, str.begin, str.size);
  return str_terminate(ret);
}

bool istr_eq(StringRange lhs, StringRange rhs) {
  if (lhs.size != rhs.size) return FALSE;
  return memcmp(lhs.begin, rhs.begin, lhs.size) == 0;
}

bool istr_starts_with(StringRange str, StringRange starts) {
  if (starts.size > str.size) return FALSE;
  return memcmp(str.begin, starts.begin, starts.size) == 0;
}

bool istr_ends_with(StringRange str, StringRange ends) {
  if (ends.size > str.size) return FALSE;
  return memcmp(str.begin + str.size - ends.size, ends.begin, ends.size) == 0;
}

bool istr_contains(StringRange str, StringRange check) {
  return istr_find(str, check) != str.size;
}

bool istr_contains_char(StringRange str, char check) {
  for (index_s i = 0; i < str.size; ++i) {
    if (str.begin[i] == check) {
      return true;
    }
  }
  return false;
}

//bool istr_contains_any(StringRange str, StringRange check_chars) {
//  // TODO: this, and istr_token, and str_ versions
//}

bool istr_to_bool(StringRange str, bool* out) {
  if (!out) return false;
  if (str.size < 4) return false;
  if (tolower(str.begin[0]) == 't') {
    for (index_s i = 1; i < 4; ++i) {
      if (tolower(str.begin[i]) != str_true->begin[i]) return false;
    }
    *out = true;
    return true;
  } else if (tolower(str.begin[0]) == 'f') {
    for (index_s i = 1; i < 5; ++i) {
      if (tolower(str.begin[i]) != str_false->begin[i]) return false;
    }
    *out = false;
    return true;
  }
  return false;
}

bool istr_to_int(StringRange str, int* out) {
  if (!out) return false;
  index_s long_out;
  bool ret = istr_to_long(str, &long_out);
  if (ret) *out = (int)long_out;
  return ret;
}

bool istr_to_long(StringRange str, index_s* out) {
  if (!out || str.size == 0) return false;
  index_s sign = 1;
  index_s start = 0;

  if (str.begin[0] == '-') {
    sign = -1;
    ++start;
  } else if (str.begin[0] == '+') {
    ++start;
  }

  index_s num = 0;
  index_s i = start;
  for (; i < str.size; ++i) {
    char c = str.begin[i];
    if (!isdigit(c)) break;
    num *= 10;
    num += c - '0';
  }

  // return false in the case of a string only containing "-" or "+";
  if (i == start) return false;

  *out = sign * num;

  return true;
}

bool istr_to_float(StringRange str, float* out) {
  if (!out) return false;
  double long_out;
  bool ret = istr_to_double(str, &long_out);
  if (ret) *out = (float)long_out;
  return ret;
}

bool istr_to_double(StringRange str, double* out) {
  assert(out);

  index_s i = 0;
  const char* s = str.begin;
  while (i < str.size && isspace(s[i])) {
    ++i;
  }

  // check for valid number start
  if (i >= str.size || (s[i] != '+' && s[i] != '-' && !isdigit(s[i]))) {
    return false;
  }

  double res = 0, fact = 1;

  // positive/negative
  if (s[i] == '-') {
    fact = -1;
    ++i;
  } else if (s[i] == '+') {
    ++i;
  }

  // digits and decimal
  for (bool in_decimal = FALSE; i < str.size; ++i) {

    if (!in_decimal && s[i] == '.') {
      in_decimal = TRUE;
      continue;
    }

    if (!isdigit(s[i])) {
      break;
    }

    int n = s[i] - '0';
    if (in_decimal) fact /= 10.0;
    res = res * 10.0 + (double)n;
  }

  *out = res * fact;
  return TRUE;
}

index_s istr_index_of_char(StringRange str, char c, index_s from_pos) {
  if (from_pos >= str.size) return str.size;
  for (index_s i = from_pos; i < str.size; ++i) {
    if (str.begin[i] == c) {
      return i;
    }
  }
  return str.size;
}

index_s istr_index_of(StringRange str, StringRange to_find, index_s from_pos) {
  if (str.size < to_find.size) return str.size;
  if (to_find.size == 0) return MIN(from_pos, str.size);
  index_s j;
  for (index_s i = from_pos; i <= str.size - to_find.size; ++i) {
    j = 0;
    while (j < to_find.size) {
      if (str.begin[i + j] != to_find.begin[j]) break;
      if (++j == to_find.size) return i;
    }
  }
  return str.size;
}

StringRange istr_token(StringRange str, StringRange to_find, index_s* pos) {
  assert(pos != NULL);
  assert(to_find.size != 0);
  if (str.size <= *pos) return str_empty->range;

  for (index_s i = *pos; i < str.size; ++i) {
    for (index_s d = 0; d < to_find.size; ++d) {
      if (to_find.begin[d] == str.begin[i]) {
        index_s old_pos = *pos;
        *pos = i + 1;
        return (StringRange) {
          .begin = str.begin + old_pos,
          .size = i - old_pos
        };
      }
    }
  }

  *pos = str.size;
  return str_empty->range;
}

index_s istr_find(StringRange str, StringRange to_find) {
  return istr_index_of(str, to_find, 0);
}

StringRange istr_substring(StringRange str, index_s start, index_s end) {
  if (start == end) return str_empty->range;
  if (start >= str.size) return str_empty->range;
  if (start < 0) start = str.size + start;
  if (start < 0) start = 0;
  if (end > str.size) end = str.size;
  if (end < 0) end = str.size + end;
  if (end <= start) return str_empty->range;
  return (StringRange) {
    .begin = str.begin + start,
    .size = end - start,
  };
}

StringRange istr_trim(StringRange str) {
  StringRange ret = istr_trim_start(str);
  return istr_trim_end(ret);
}

StringRange istr_trim_start(StringRange str) {
  index_s start, end = str.size;
  for (start = 0; start < str.size; ++start) {
    if (!isspace(str.begin[start])) break;
  }
  if (start == end) return str_empty->range;
  return (StringRange) {
    .begin = str.begin + start,
      .size = end - start,
  };
}

StringRange istr_trim_end(StringRange str) {
  index_s end = str.size;
  while (end > 0) {
    if (isspace(str.begin[end - 1])) --end;
    else break;
  }
  if (end <= 0) return str_empty->range;
  return (StringRange) {
    .begin = str.begin,
      .size = end,
  };
}

Array_StrR istr_split(StringRange str, StringRange del) {
  Array_StrR ret = arr_str_new();

  // specialization for empty delimiter, return a range for each char
  if (del.size == 0) {
    arr_str_reserve(ret, str.size);
    for (index_s i = 0; i < str.size; ++i) {
      StringRange c = str_range_s(&str.begin[i], 1);
      arr_str_push_back(ret, c);
    }
    return ret;
  }

  index_s i = 0;
  do {
    index_s prev = i;
    i = istr_index_of(str, del, i);
    StringRange range = istr_substring(str, prev, i);
    arr_str_push_back(ret, range);
    i += del.size;
    if (i == str.size) arr_str_push_back(ret, str_empty->range);
  } while (i < str.size);

  return ret;
}

String istr_join(StringRange del, const Array_StringRange strings) {
  const index_s range_count = strings->size;
  if (range_count == 0) return str_empty;

  index_s length = 0;

  StringRange* array_foreach(range, strings) {
    length += range->size;
  }

  length += del.size * (range_count - 1);

  String_Internal* ret = str_new_internal(length);
  if (!ret) return str_empty;

  char* dst = ret->begin;

  array_foreach_index(range, i, strings) {
    memcpy(dst, range->begin, range->size);
    dst += range->size;

    if (i != range_count - 1) {
      memcpy(dst, del.begin, del.size);
      dst += del.size;
    }
  }

  return str_terminate(ret);
}

String istr_concat(StringRange left, StringRange right) {
  index_s length = left.size + right.size;
  String_Internal* ret = str_new_internal(length);
  if (!ret) return str_empty;
  memcpy(ret->begin, left.begin, left.size);
  memcpy(ret->begin + left.size, right.begin, right.size);
  return str_terminate(ret);
}

String istr_prepend(StringRange str, index_s length, char c) {
  String_Internal* ret = str_new_internal(str.size + length);
  memset(ret->begin, c, length);
  memcpy(ret->begin + length, str.begin, str.size);
  return str_terminate(ret);
}

String istr_append(StringRange str, index_s length, char c) {
  String_Internal* ret = str_new_internal(str.size + length);
  memcpy(ret->begin, str.begin, str.size);
  memset(ret->begin + str.size, c, length);
  return str_terminate(ret);
}

////////////////////////////////////////////////////////////////////////////////
// str_format
////////////////////////////////////////////////////////////////////////////////

// TODO: ? (not so useful when requiring ranges. Maybe make something
//          like this for string builder?)
// String _str_format(StringRange fmt, const StringRange[] argv, uint argc);
// #define _str_format_va(fmt, argv, argc, ...) _str_format(fmt, argv, argc)
// #define str_format(...) _str_fmt2(__VA_ARGS__, NULL, 0)
// TODO: verify that non-macro va args are actaully not a thing with wasi, lol

#define con_type byte
#include "array.h"
#undef con_type

const _Str_FmtArg _str_fmtarg_end = { .type = _Str_FmtArg_End, .i = 0 };

enum _Str_FmtSpec_Alignment {
  _Str_FmtSpec_Left,
  _Str_FmtSpec_Center,
  _Str_FmtSpec_Right
};

typedef enum {
  _Str_FmtState_Index,
  _Str_FmtState_Style,
  _Str_FmtState_Flags,
  _Str_FmtState_Padch,
  _Str_FmtState_Width,
  _Str_FmtState_Preci,
  _Str_FmtState_Final
} _Str_FmtState;

typedef enum {
  _Str_FmtAlign_Left,
  _Str_FmtAlign_Center,
  _Str_FmtAlign_Right,
  _Str_FmtAlign_Right_LeftSign,
} _Str_FmtAlign;

typedef enum {
  _Str_FmtRep_Default,
  _Str_FmtRep_Hex,
  _Str_FmtRep_HEX,
  _Str_FmtRep_Binary,
  _Str_FmtRep_Char,
  _Str_FmtRep_Day,
  _Str_FmtRep_DayShort,
  _Str_FmtRep_Month,
  _Str_FmtRep_MonthShort
} _Str_FmtRep;

const byte _str_fmtarg_invalid_spec = 255;

typedef struct {
  byte index;
  byte padding;             // default = space
  byte precision;           // default of 1 (ie, 1.0)
  byte alignment : 2;       // 0 = left, 1 = center, 2 = right
  byte sign : 1;            // 0 = - only, 1 = + for positive numbers
  byte representation : 4;  // 0 = default, 1 = hex, 2 = char, 3 = binary, d, D, m, M
  byte trailing : 1;        // 0 = don't, 1 = do ; only for decimals
  byte sci_notation : 2;    // 0 = no, 1 = e, 2 = E
  byte percentage : 1;      // 0 = no, 1 = display decimals as percent
  ushort width;             // 0 = no padding
} _Str_FmtSpec;

// handle format specifier
// {[index][:(+)(<^>)(width)(.precision[+])]}
// basic: "values: {}, {}, {}",                   1, 2, "hi"-> "values: 1, 2, hi"
// index: "values: {2}, {1}, {0}",                1, 2, 3   -> "values: 3, 2, 1"
// width: "values: {:4}, {0:4}",                  1         -> "values: 1   , 1   "
// align: "values: |{0:<4}|{0:>4}|{0:^4}|{0:^5}", 1         -> "values: |1   |   1| 1  |  1  |"
// preci: "values: |{0:.5}|{0:^.1}|{0:>10.6+}|",  1.23      -> "values: |1.23|1.2|1.230000  |"
// types: "values: {0!i} {0!x} {0!c}",            65        -> "values: 65, 41, A"
// date format specifiers? (ie, {%d} for "Monday" (use standard)
static _Str_FmtSpec format_read_spec(
  StringRange spec_str, byte arg_index, index_s* spec_end
) {

  // set up the format specifier struct
  _Str_FmtSpec spec = { 0 };
  spec.index = arg_index;

  _Str_FmtState read_state = _Str_FmtState_Index;

  for (index_s i = 0; i < spec_str.size; ++i) {
    char c = spec_str.begin[i];

    // close out the specifier at any point, also handles "{}"
    if (c == '}') {
      *spec_end = i + 1;
      if (spec.precision == 0) {
        spec.precision = 1;
      }
      if (spec.padding == 0) {
        spec.padding = ' ';
      }
      return spec;
    }

    switch (read_state) {

      // reading the arg index: "{12}", "{12!b}", "{0:10}"
      case _Str_FmtState_Index: {

        if (c == '!') {
          read_state = _Str_FmtState_Style;
          break;
        }

        if (c == ':') {
          read_state = _Str_FmtState_Flags;
          break;
        }

        // invalid if index is over 99 or contains non-digit
        if (i >= 2 || !isdigit(c)) {
          goto invalid_spec;
        }

        if (i == 0) spec.index = 0;
        spec.index *= 10;
        spec.index += c - '0';

      } break;

      // reading conversions: "{!x}", "{1!c}", "{!M:10}"
      case _Str_FmtState_Style: {

        switch (c) {
          case ':': read_state = _Str_FmtState_Flags; break;
          case 'i': spec.representation = _Str_FmtRep_Default; break;
          case 'f': spec.representation = _Str_FmtRep_Default; break;
          case 'x': spec.representation = _Str_FmtRep_Hex; break;
          case 'X': spec.representation = _Str_FmtRep_HEX; break;
          case 'b': spec.representation = _Str_FmtRep_Binary; break;
          case 'c': spec.representation = _Str_FmtRep_Char; break;
          case 'D': spec.representation = _Str_FmtRep_Day; break;
          case 'd': spec.representation = _Str_FmtRep_DayShort; break;
          case 'M': spec.representation = _Str_FmtRep_Month; break;
          case 'm': spec.representation = _Str_FmtRep_MonthShort; break;
          default: goto invalid_spec;
        }

      } break;

      // reading the pad character in the flags section "{:#X10}"
      case _Str_FmtState_Padch: {

        if (spec.padding) {
          goto invalid_spec;
        }

        spec.padding = c;
        read_state = _Str_FmtState_Flags;

      } break;

      // reading the prefix flag chars: "{:+<^>#_}"
      case _Str_FmtState_Flags: {

        switch (c) {
          case '+': spec.sign = 1; break;
          case '<': spec.alignment = _Str_FmtAlign_Left; break;
          case '^': spec.alignment = _Str_FmtAlign_Center; break;
          case '>': spec.alignment = _Str_FmtAlign_Right; break;
          case '=': spec.alignment = _Str_FmtAlign_Right_LeftSign; break;
          case '#': read_state = _Str_FmtState_Padch; break;
          case '.': read_state = _Str_FmtState_Preci; break;
          default: {
            if (!isdigit(c)) {
              goto invalid_spec;
            }
            read_state = _Str_FmtState_Width;
          }
        }

        if (read_state != _Str_FmtState_Width) break;

      } SWITCH_FALLTHROUGH; // fallthrough for digits

      // reading width specifier: "{:10}", "{:010}", "{:10.5}"
      case _Str_FmtState_Width: {

        if (c == '.') {
          read_state = _Str_FmtState_Preci;
          break;
        }

        if (!isdigit(c)) {
          goto invalid_spec;
        }

        if (c == '0' && spec.width == 0) {
          if (!spec.padding) spec.padding = '0';
          spec.alignment = _Str_FmtAlign_Right_LeftSign;
          break;
        }

        spec.width *= 10;
        spec.width += c - '0';

      } break;

      // precision for decimals: "{:10.5}", "{:10.5+}", "{:.4e}"
      case _Str_FmtState_Preci: {

        if (c == '+') {
          spec.trailing = 1;
          read_state = _Str_FmtState_Final;
          break;
        }

        if (spec.sci_notation == 0) {
          if (c == 'e') { spec.sci_notation = 1; break; }
          if (c == 'E') { spec.sci_notation = 2; break; }
        }

        if (!isdigit(c) || spec.sci_notation != 0) {
          goto invalid_spec;
        }

        spec.precision *= 10;
        spec.precision += c - '0';

      } break;

      // in this state, we've read all of the format, it's } or bust.
      case _Str_FmtState_Final: {
        goto invalid_spec;
      }

    }

  }

invalid_spec:

  spec.index = _str_fmtarg_invalid_spec;
  return spec;
}

static void format_print_arg_align_number(
  Array_byte out, _Str_FmtSpec spec, index_s excess, index_s start, index_s msd
) {
  if (!excess) return;

  switch (spec.alignment) {

    case _Str_FmtAlign_Left: {
      byte* pad = arr_byte_emplace_back_range(out, excess);
      memset(pad, spec.padding, excess);
    } break;

    case _Str_FmtAlign_Center: {
      index_s half = (excess + 1) / 2;
      index_s back_size = excess - half;
      byte* pad = arr_byte_emplace_range(out, start, half);
      memset(pad, spec.padding, half);
      pad = arr_byte_emplace_back_range(out, back_size);
      memset(pad, spec.padding, back_size);
    } break;

    case _Str_FmtAlign_Right: {
      byte* pad = arr_byte_emplace_range(out, start, excess);
      memset(pad, spec.padding, excess);
    } break;

    case _Str_FmtAlign_Right_LeftSign: {
      byte* pad = arr_byte_emplace_range(out, msd, excess);
      memset(pad, spec.padding, excess);
    } break;

  }

}

static void format_print_arg_int(
  Array_byte out, _Str_FmtSpec spec, ptrdiff_t i
) {

  switch (spec.representation) {

    case _Str_FmtRep_Char: {
      arr_byte_push_back(out,
        (i <= 0x1F || i == 0x7F || i > 127) ? '.' : (byte)i
      );
    } break;

    case _Str_FmtRep_Default: {
      do {
        ptrdiff_t digit = i % 10;
        arr_byte_push_back(out, (byte)(digit + '0'));
        i /= 10;
      } while (i);
    } break;

    case _Str_FmtRep_Hex:
    case _Str_FmtRep_HEX: {
      do {
        ptrdiff_t digit = i % 16;
        byte c = spec.representation == _Str_FmtRep_HEX ? 'A' : 'a';
        byte b = (byte)digit + (digit >= 10 ? c-10 : '0');
        arr_byte_push_back(out, b);
        i /= 16;
      } while (i);
    } break;

    case _Str_FmtRep_Binary: {
      do {
        ptrdiff_t digit = i % 2;
        arr_byte_push_back(out, (byte)(digit + '0'));
        i /= 2;
      } while (i);
    } break;

  }

}

static void format_print_arg_float(
  Array_byte out, _Str_FmtSpec spec, double f_val, index_s start
) {

  double f = f_val;
  do {
    ptrdiff_t digit = (int)f;
    digit %= 10;
    f /= 10;
    arr_byte_push_back(out, (byte)(digit + '0'));
  } while (f >= 1.0);

  byte* digits = arr_byte_ref(out, start);
  memrev(digits, (uint)(out->size - start));

  f = f_val - floor(f_val);
  byte p = spec.precision;

  if (f != 0.0 && spec.precision) {
    arr_byte_push_back(out, '.');
    for (; p && f > 0.00000000001; --p) {
      f *= 10.0;
      int int_part = (int)f;
      arr_byte_push_back(out, (byte)(int_part + '0'));
      f -= int_part;
    }
  } else if (spec.precision && spec.trailing) {
    arr_byte_push_back(out, '.');
  }

  while (p && spec.trailing) {
    arr_byte_push_back(out, '0');
    --p;
  }

}

static void format_print_arg(Array_byte out, Array params, _Str_FmtSpec spec) {

  if (spec.index >= params->size) {

    // special case for padding an out-of-bounds argument
    if (spec.width) {
      byte* bytes = arr_byte_emplace_back_range(out, spec.width);
      memset(bytes, spec.padding, spec.width);
    }

    return;
  }

  _Str_FmtArg* arg = array_ref(params, spec.index);

  switch (arg->type) {

    case _Str_FmtArg_StringRange: {

      index_s width = MAX(spec.width, arg->range.size);
      index_s excess = width - arg->range.size;

      byte* bytes = arr_byte_emplace_back_range(out, width);

      index_s pos = 0;
      if (excess) {

        switch (spec.alignment) {

          case _Str_FmtAlign_Left: {
            memset(bytes + arg->range.size, spec.padding, excess);
          } break;

          case _Str_FmtAlign_Center: {
            pos = (excess + 1) / 2;
            index_s back_size = width - arg->range.size - pos;
            memset(bytes, spec.padding, pos);
            memset(bytes + pos + arg->range.size, spec.padding, back_size);
          } break;

          case _Str_FmtAlign_Right:
          case _Str_FmtAlign_Right_LeftSign: {
            pos = excess;
            memset(bytes, spec.padding, excess);
          } break;

        }

      }

      memcpy(bytes + pos, arg->range.begin, arg->range.size);

    } break;

    case _Str_FmtArg_Int: {

      ptrdiff_t i = arg->i;
      index_s start = out->size;

      if (i < 0) {
        arr_byte_push_back(out, '-');
        i *= -1;
      } else if (spec.sign) {
        arr_byte_push_back(out, '+');
      }

      index_s msd = out->size;

      format_print_arg_int(out, spec, i);

      byte* digits = arr_byte_ref(out, msd);
      memrev(digits, (uint)(out->size - msd));

      index_s width = MAX(spec.width, out->size - start);
      index_s excess = width - (out->size - start);

      format_print_arg_align_number(out, spec, excess, start, msd);

    } break;

    case _Str_FmtArg_Float: {

      double f = arg->f;
      index_s start = out->size;

      if (f < 0) {
        arr_byte_push_back(out, '-');
        f *= -1;
      } else if (spec.sign) {
        arr_byte_push_back(out, '+');
      }

      index_s msd = out->size;

      format_print_arg_float(out, spec, f, msd);

      index_s width = MAX(spec.width, out->size - start);
      index_s excess = width - (out->size - start);

      format_print_arg_align_number(out, spec, excess, start, msd);

    } break;

    default: {

      byte* bytes = arr_byte_emplace_back_range(out, 22);
      memcpy(bytes, " <can't resolve type> ", 22);

    } break;

  } 

}

static String format_va(StringRange fmt, va_list args) {
  // TODO: better error handling on memory errors
  // TODO: Move this to its own section, possibly want to split out a new 
  //    header just for this function, especially if other dependent types
  //    end up being supported (such as vec3).
  Array params = array_new(_Str_FmtArg);
  index_s reserve_size = sizeof(String_Internal) + fmt.size;

  _Str_FmtArg arg;

  loop{
    arg = va_arg(args, _Str_FmtArg);

    until(arg.type == _Str_FmtArg_End);

    reserve_size += (arg.type == _Str_FmtArg_StringRange) ? arg.range.size : 3;

    array_write_back(params, &arg);
  }

  va_end(args);

  // Set the starting allocation for the new string
  Array_byte output = arr_byte_new_reserve(reserve_size);

  // Push the String header to the front of the array data
  if (!arr_byte_emplace_back_range(output, sizeof(struct _Str_Base))) {
    return str_empty;
  }

  // Process the format string
  byte arg_index = 0;

  // track position in each section between formatters
  index_s section_start = 0;
  index_s section_size = 0;

  for (index_s i = 0; i < fmt.size; ++i) {
    byte c = fmt.begin[i];

    // just do a 1:1 copy by character until we hit a format specifier
    if (c != '{') {
      ++section_size;
      continue;
    }

    // at the start of a format section, copy all the bytes up to this point
    if (section_size) {
      byte* bytes = arr_byte_emplace_back_range(output, section_size);
      memcpy(bytes, fmt.begin + section_start, section_size);
    }

    section_size = 0;

    // handle case for "{{" to print escaped left brace
    if (fmt.begin[i + 1] == '{') {
      section_start = ++i;
      section_size = 1;
      continue;
    }

    index_s spec_end;
    StringRange spec_str = istr_substring(fmt, i + 1, fmt.size);
    _Str_FmtSpec spec = format_read_spec(spec_str, arg_index, &spec_end);

    // rather than error on invalid spec, just print the characters
    // this means we don't need to worry about escaping braces most of the time
    if (spec.index == _str_fmtarg_invalid_spec) {
      section_start = i;
      section_size = 1;
      continue;
    }

    arg_index = spec.index + 1;

    format_print_arg(output, params, spec);

    i += spec_end;
    section_start = i + 1;
  }

  // if we reach the end and we were reading chars for output, print them here
  if (section_size) {
    byte* bytes = arr_byte_emplace_back_range(output, section_size);
    memcpy(bytes, fmt.begin + section_start, section_size);
  }

  array_delete(&params);

  arr_byte_push_back(output, '\0');
  String_Internal* header = (String_Internal*)output->arr;
  header->size = output->size - sizeof(struct _Str_Base) - 1;
  header->begin = &header->head;
  arr_byte_truncate(output, output->size);
  String ret = (String)arr_byte_release(&output);

  return ret;
}

String istr_format(StringRange fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return format_va(fmt, args);
}

void istr_print(StringRange fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String to_print = format_va(fmt, args);
  if (to_print == NULL) return;
  istr_write(to_print->range);
  str_delete(&to_print);
}

void istr_log(StringRange fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String to_print = format_va(fmt, args);
  if (to_print == NULL) return;
  istr_write(to_print->range);
  str_delete(&to_print);
}

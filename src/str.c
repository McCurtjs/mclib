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
#include "utility.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#undef SRCV
#define SRCV
typedef struct {
  // public (read-only)
  union {
    slice_t slice;
    struct {
      char* begin;
      union {
        index_t length;
        index_t size;
      };
    };
  };

  // private
  char head;
} String_Internal;

extern char slice_constants[];

static struct slice_t str_constants[] = {
  { .begin = &slice_constants[0], .size = 0 },
  { .begin = &slice_constants[0], .size = 0 },
  { .begin = &slice_constants[1], .size = 4 },
  { .begin = &slice_constants[6], .size = 5 },
};

static String const str_constants_begin = (String)&str_constants[0];
static String const str_constants_end = (String)&str_constants[0] + ARRAY_COUNT(str_constants);

const String str_empty  = (String)&str_constants[0];
const String str_va_end = (String)&str_constants[1];
const String str_true   = (String)&str_constants[2];
const String str_false  = (String)&str_constants[3];

static String_Internal* str_new_internal(index_t length) {
  if (length <= 0) return NULL; // prompt callers to return empty string
  // Include an extra byte for the null terminator
  String_Internal* ret = malloc(sizeof(slice_t) + length + 1);
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
  return (str >= str_constants_begin && str < str_constants_end);
}

////////////////////////////////////////////////////////////////////////////////
// Direct string str_ functions
////////////////////////////////////////////////////////////////////////////////

String str_new(const char* c_str) {
  if (!c_str) return str_empty;
  index_t length = strlen(c_str);
  String_Internal* ret = str_new_internal(length);
  if (!ret) return str_empty;
  memcpy(&ret->head, c_str, length);
  return str_terminate(ret);
}

String str_new_s(const char* c_str, index_t length) {
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

bool str_is_null_or_empty(const String str) {
  if (!str) return true;
  return slice_is_empty(str->slice);
}

////////////////////////////////////////////////////////////////////////////////
// Macro-overloaded str_ via istr_ functions
////////////////////////////////////////////////////////////////////////////////

String istr_copy(slice_t str) {
  String_Internal* ret = str_new_internal(str.size);
  if (!ret) return str_empty;
  memcpy(ret->begin, str.begin, str.size);
  return str_terminate(ret);
}

//bool istr_contains_any(slice_t str, slice_t check_chars) {
//  // TODO: this, and istr_token, and str_ versions
//}

String istr_join(slice_t del, const Array_slice strings) {
  const index_t slice_count = strings->size;
  if (slice_count == 0) return str_empty;

  index_t length = 0;

  slice_t* array_foreach(slice, strings) {
    length += slice->size;
  }

  length += del.size * (slice_count - 1);

  String_Internal* ret = str_new_internal(length);
  if (!ret) return str_empty;

  char* dst = ret->begin;

  array_foreach_index(slice, i, strings) {
    memcpy(dst, slice->begin, slice->size);
    dst += slice->size;

    if (i != slice_count - 1) {
      memcpy(dst, del.begin, del.size);
      dst += del.size;
    }
  }

  return str_terminate(ret);
}

String istr_concat(slice_t left, slice_t right) {
  index_t length = left.size + right.size;
  String_Internal* ret = str_new_internal(length);
  if (!ret) return str_empty;
  memcpy(ret->begin, left.begin, left.size);
  memcpy(ret->begin + left.size, right.begin, right.size);
  return str_terminate(ret);
}

String istr_prepend(slice_t str, index_t length, char c) {
  String_Internal* ret = str_new_internal(str.size + length);
  memset(ret->begin, c, length);
  memcpy(ret->begin + length, str.begin, str.size);
  return str_terminate(ret);
}

String istr_append(slice_t str, index_t length, char c) {
  String_Internal* ret = str_new_internal(str.size + length);
  memcpy(ret->begin, str.begin, str.size);
  memset(ret->begin + str.size, c, length);
  return str_terminate(ret);
}

////////////////////////////////////////////////////////////////////////////////
// str_format
////////////////////////////////////////////////////////////////////////////////

// TODO: ? (not so useful when requiring slices. Maybe make something
//          like this for string builder?)
// String _str_format(slice_t fmt, const slice_t[] argv, uint argc);
// #define _str_format_va(fmt, argv, argc, ...) _str_format(fmt, argv, argc)
// #define str_format(...) _str_fmt2(__VA_ARGS__, NULL, 0)
// TODO: verify that non-macro va args are actaully not a thing with wasi, lol

#define con_skip_dependencies
#include "array_byte.h"

const _str_fmtArg_t _str_fmtarg_end = { .type = _fmtArg_End, .i = 0 };

enum _fmtSpec_alignment_t {
  _fmtSpec_Left,
  _fmtSpec_Center,
  _fmtSpec_Right
};

typedef enum {
  _str_fmtState_Index,
  _str_fmtState_Style,
  _str_fmtState_Flags,
  _str_fmtState_Padch,
  _str_fmtState_Width,
  _str_fmtState_Preci,
  _str_fmtState_Final
} _str_fmtState_t;

typedef enum {
  _str_fmtAlign_Left,
  _str_fmtAlign_Center,
  _str_fmtAlign_Right,
  _str_fmtAlign_Right_LeftSign,
} _str_fmtAlign_t;

typedef enum {
  _str_fmtRep_Default,
  _str_fmtRep_Hex,
  _str_fmtRep_HEX,
  _str_fmtRep_Binary,
  _str_fmtRep_Char,
  _str_fmtRep_Day,
  _str_fmtRep_DayShort,
  _str_fmtRep_Month,
  _str_fmtRep_MonthShort
} _str_fmtRep_t;

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
  slice_t spec_str, byte arg_index, index_t* spec_end
) {

  // set up the format specifier struct
  _Str_FmtSpec spec = { 0 };
  spec.index = arg_index;

  _str_fmtState_t read_state = _str_fmtState_Index;

  for (index_t i = 0; i < spec_str.size; ++i) {
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
      case _str_fmtState_Index: {

        if (c == '!') {
          read_state = _str_fmtState_Style;
          break;
        }

        if (c == ':') {
          read_state = _str_fmtState_Flags;
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
      case _str_fmtState_Style: {

        switch (c) {
          case ':': read_state = _str_fmtState_Flags; break;
          case 'i': spec.representation = _str_fmtRep_Default; break;
          case 'f': spec.representation = _str_fmtRep_Default; break;
          case 'x': spec.representation = _str_fmtRep_Hex; break;
          case 'X': spec.representation = _str_fmtRep_HEX; break;
          case 'b': spec.representation = _str_fmtRep_Binary; break;
          case 'c': spec.representation = _str_fmtRep_Char; break;
          case 'D': spec.representation = _str_fmtRep_Day; break;
          case 'd': spec.representation = _str_fmtRep_DayShort; break;
          case 'M': spec.representation = _str_fmtRep_Month; break;
          case 'm': spec.representation = _str_fmtRep_MonthShort; break;
          default: goto invalid_spec;
        }

      } break;

      // reading the pad character in the flags section "{:#X10}"
      case _str_fmtState_Padch: {

        if (spec.padding) {
          goto invalid_spec;
        }

        spec.padding = c;
        read_state = _str_fmtState_Flags;

      } break;

      // reading the prefix flag chars: "{:+<^>#_}"
      case _str_fmtState_Flags: {

        switch (c) {
          case '+': spec.sign = 1; break;
          case '<': spec.alignment = _str_fmtAlign_Left; break;
          case '^': spec.alignment = _str_fmtAlign_Center; break;
          case '>': spec.alignment = _str_fmtAlign_Right; break;
          case '=': spec.alignment = _str_fmtAlign_Right_LeftSign; break;
          case '#': read_state = _str_fmtState_Padch; break;
          case '.': read_state = _str_fmtState_Preci; break;
          default: {
            if (!isdigit(c)) {
              goto invalid_spec;
            }
            read_state = _str_fmtState_Width;
          }
        }

        if (read_state != _str_fmtState_Width) break;

      } SWITCH_FALLTHROUGH; // fallthrough for digits

      // reading width specifier: "{:10}", "{:010}", "{:10.5}"
      case _str_fmtState_Width: {

        if (c == '.') {
          read_state = _str_fmtState_Preci;
          break;
        }

        if (!isdigit(c)) {
          goto invalid_spec;
        }

        if (c == '0' && spec.width == 0) {
          if (!spec.padding) spec.padding = '0';
          spec.alignment = _str_fmtAlign_Right_LeftSign;
          break;
        }

        spec.width *= 10;
        spec.width += c - '0';

      } break;

      // precision for decimals: "{:10.5}", "{:10.5+}", "{:.4e}"
      case _str_fmtState_Preci: {

        if (c == '+') {
          spec.trailing = 1;
          read_state = _str_fmtState_Final;
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
      case _str_fmtState_Final: {
        goto invalid_spec;
      }

    }

  }

invalid_spec:

  spec.index = _str_fmtarg_invalid_spec;
  return spec;
}

static void format_print_arg_align_number(
  Array_byte out, _Str_FmtSpec spec, index_t excess, index_t start, index_t msd
) {
  if (!excess) return;

  switch (spec.alignment) {

    case _str_fmtAlign_Left: {
      //* pad = arr_byte_emplace_back_slice(out, excess).begin;
      //memset(pad, spec.padding, excess);
      span_byte_t pad = arr_byte_emplace_back_range(out, excess);
      span_byte_set_bytes(pad, spec.padding);
    } break;

    case _str_fmtAlign_Center: {
      index_t half = (excess + 1) / 2;
      index_t back_size = excess - half;
      span_byte_t pad = arr_byte_emplace_range(out, start, half);
      span_byte_set_bytes(pad, spec.padding);
      pad = arr_byte_emplace_back_range(out, back_size);
      span_byte_set_bytes(pad, spec.padding);
    } break;

    case _str_fmtAlign_Right: {
      span_byte_t pad = arr_byte_emplace_range(out, start, excess);
      span_byte_set_bytes(pad, spec.padding);
    } break;

    case _str_fmtAlign_Right_LeftSign: {
      span_byte_t pad = arr_byte_emplace_range(out, msd, excess);
      span_byte_set_bytes(pad, spec.padding);
    } break;

  }

}

static void format_print_arg_int(
  Array_byte out, _Str_FmtSpec spec, ptrdiff_t i
) {

  switch (spec.representation) {

    case _str_fmtRep_Char: {
      arr_byte_push_back(out,
        (i <= 0x1F || i == 0x7F || i > 127) ? '.' : (byte)i
      );
    } break;

    case _str_fmtRep_Default: {
      do {
        ptrdiff_t digit = i % 10;
        arr_byte_push_back(out, (byte)(digit + '0'));
        i /= 10;
      } while (i);
    } break;

    case _str_fmtRep_Hex:
    case _str_fmtRep_HEX: {
      do {
        ptrdiff_t digit = i % 16;
        byte c = spec.representation == _str_fmtRep_HEX ? 'A' : 'a';
        byte b = (byte)digit + (digit >= 10 ? c-10 : '0');
        arr_byte_push_back(out, b);
        i /= 16;
      } while (i);
    } break;

    case _str_fmtRep_Binary: {
      do {
        ptrdiff_t digit = i % 2;
        arr_byte_push_back(out, (byte)(digit + '0'));
        i /= 2;
      } while (i);
    } break;

  }

}

static void format_print_arg_float(
  Array_byte out, _Str_FmtSpec spec, double f_val, index_t start
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
      span_byte_t bytes = arr_byte_emplace_back_range(out, spec.width);
      span_byte_set_bytes(bytes, spec.padding);
    }

    return;
  }

  _str_fmtArg_t* arg = array_ref(params, spec.index);

  switch (arg->type) {

    case _fmtArg_Slice: {

      index_t width = MAX(spec.width, arg->slice.size);
      index_t excess = width - arg->slice.size;

      byte* bytes = arr_byte_emplace_back_range(out, width).begin;

      index_t pos = 0;
      if (excess) {

        switch (spec.alignment) {

          case _str_fmtAlign_Left: {
            memset(bytes + arg->slice.size, spec.padding, excess);
          } break;

          case _str_fmtAlign_Center: {
            pos = (excess + 1) / 2;
            index_t back_size = width - arg->slice.size - pos;
            memset(bytes, spec.padding, pos);
            memset(bytes + pos + arg->slice.size, spec.padding, back_size);
          } break;

          case _str_fmtAlign_Right:
          case _str_fmtAlign_Right_LeftSign: {
            pos = excess;
            memset(bytes, spec.padding, excess);
          } break;

        }

      }

      memcpy(bytes + pos, arg->slice.begin, arg->slice.size);

    } break;

    case _fmtArg_Int: {

      ptrdiff_t i = arg->i;
      index_t start = out->size;

      if (i < 0) {
        arr_byte_push_back(out, '-');
        i *= -1;
      } else if (spec.sign) {
        arr_byte_push_back(out, '+');
      }

      index_t msd = out->size;

      format_print_arg_int(out, spec, i);

      span_byte_t digits = span_byte(out->begin + msd, out->end);
      span_byte_reverse(digits);

      index_t width = MAX(spec.width, out->size - start);
      index_t excess = width - (out->size - start);

      format_print_arg_align_number(out, spec, excess, start, msd);

    } break;

    case _fmtArg_Float: {

      double f = arg->f;
      index_t start = out->size;

      if (f < 0) {
        arr_byte_push_back(out, '-');
        f *= -1;
      } else if (spec.sign) {
        arr_byte_push_back(out, '+');
      }

      index_t msd = out->size;

      format_print_arg_float(out, spec, f, msd);

      index_t width = MAX(spec.width, out->size - start);
      index_t excess = width - (out->size - start);

      format_print_arg_align_number(out, spec, excess, start, msd);

    } break;

    default: {

      byte* bytes = arr_byte_emplace_back_range(out, 22).begin;
      memcpy(bytes, " <can't resolve type> ", 22);

    } break;

  } 

}

static String format_va(slice_t fmt, va_list args) {
  // TODO: better error handling on memory errors
  // TODO: Move this to its own section, possibly want to split out a new 
  //    header just for this function, especially if other dependent types
  //    end up being supported (such as vec3).
  Array params = array_new(_str_fmtArg_t);
  index_t reserve_size = sizeof(String_Internal) + fmt.size;

  _str_fmtArg_t arg;

  loop{
    arg = va_arg(args, _str_fmtArg_t);

    until(arg.type == _fmtArg_End);

    reserve_size += (arg.type == _fmtArg_Slice) ? arg.slice.size : 3;

    array_write_back(params, &arg);
  }

  va_end(args);

  // Set the starting allocation for the new string
  Array_byte output = arr_byte_new_reserve(reserve_size);

  // Push the String header to the front of the array data
  arr_byte_emplace_back_range(output, sizeof(slice_t));

  // Process the format string
  byte arg_index = 0;

  // track position in each section between formatters
  index_t section_start = 0;
  index_t section_size = 0;

  for (index_t i = 0; i < fmt.size; ++i) {
    byte c = fmt.begin[i];

    // just do a 1:1 copy by character until we hit a format specifier
    if (c != '{') {
      ++section_size;
      continue;
    }

    // at the start of a format section, copy all the bytes up to this point
    if (section_size) {
      span_byte_t bytes = arr_byte_emplace_back_range(output, section_size);
      memcpy(bytes.begin, fmt.begin + section_start, section_size);
    }

    section_size = 0;

    // handle case for "{{" to print escaped left brace
    if (fmt.begin[i + 1] == '{') {
      section_start = ++i;
      section_size = 1;
      continue;
    }

    index_t spec_end;
    slice_t spec_str = islice_substring(fmt, i + 1, fmt.size);
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
    span_byte_t bytes = arr_byte_emplace_back_range(output, section_size);
    memcpy(bytes.begin, fmt.begin + section_start, section_size);
  }

  array_delete(&params);

  arr_byte_push_back(output, '\0');
  String_Internal* header = (String_Internal*)output->arr;
  header->size = output->size - sizeof(struct slice_t) - 1;
  header->begin = &header->head;
  arr_byte_truncate(output, output->size);
  String ret = (String)arr_byte_release(&output).begin;
  return ret;
}

String istr_format(slice_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  return format_va(fmt, args);
}

void istr_print(slice_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String to_print = format_va(fmt, args);
  if (to_print == NULL) return;
  slice_write(to_print->slice);
  str_delete(&to_print);
}

void istr_log(slice_t fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String to_print = format_va(fmt, args);
  if (to_print == NULL) return;
  slice_write(to_print->slice);
  str_delete(&to_print);
}

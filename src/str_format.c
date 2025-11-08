/*******************************************************************************
* MIT License
*
* Copyright (c) 2025 Curtis McCoy
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
#include "vec.h"

#include "str.h"

#include "utility.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "array_byte.h"

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
  char head[];
} String_Internal;

typedef enum {
  _fmt_state_index,
  _fmt_state_style,
  _fmt_state_flags,
  _fmt_state_padch,
  _fmt_state_width,
  _fmt_state_preci,
  _fmt_state_final
} _fmt_state_t;

typedef enum {
  _fmt_align_left,
  _fmt_align_center,
  _fmt_align_right,
  _fmt_align_right_leftsign,
} _fmt_align_t;

typedef enum {
  _fmt_rep_default,
  _fmt_rep_hex,
  _fmt_rep_HEX,
  _fmt_rep_binary,
  _fmt_rep_char,
  _fmt_rep_day,
  _fmt_rep_day_short,
  _fmt_rep_month,
  _fmt_rep_month_short
} _fmt_rep_t;

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
} _fmt_spec_t;

// handle format specifier
// {[index][:(+)(<^>)(width)(.precision[+])]}
// basic: "values: {}, {}, {}",                   1, 2, "hi"-> "values: 1, 2, hi"
// index: "values: {2}, {1}, {0}",                1, 2, 3   -> "values: 3, 2, 1"
// width: "values: {:4}, {0:4}",                  1         -> "values: 1   , 1   "
// align: "values: |{0:<4}|{0:>4}|{0:^4}|{0:^5}", 1         -> "values: |1   |   1| 1  |  1  |"
// preci: "values: |{0:.5}|{0:^.1}|{0:>10.6+}|",  1.23      -> "values: |1.23|1.2|1.230000  |"
// types: "values: {0!i} {0!x} {0!c}",            65        -> "values: 65, 41, A"
// date format specifiers? (ie, {%d} for "Monday" (use standard)
static _fmt_spec_t _format_read_spec(
  slice_t spec_str, byte arg_index, index_t* spec_end
) {
  // set up the format specifier struct
  _fmt_spec_t spec = { 0 };
  spec.index = arg_index;

  _fmt_state_t read_state = _fmt_state_index;

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
      case _fmt_state_index: {

        if (c == '!') {
          read_state = _fmt_state_style;
          break;
        }

        if (c == ':') {
          read_state = _fmt_state_flags;
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
      case _fmt_state_style: {

        switch (c) {
          case ':': read_state = _fmt_state_flags; break;
          case 'i': spec.representation = _fmt_rep_default; break;
          case 'f': spec.representation = _fmt_rep_default; break;
          case 'x': spec.representation = _fmt_rep_hex; break;
          case 'X': spec.representation = _fmt_rep_HEX; break;
          case 'b': spec.representation = _fmt_rep_binary; break;
          case 'c': spec.representation = _fmt_rep_char; break;
          case 'D': spec.representation = _fmt_rep_day; break;
          case 'd': spec.representation = _fmt_rep_day_short; break;
          case 'M': spec.representation = _fmt_rep_month; break;
          case 'm': spec.representation = _fmt_rep_month_short; break;
          default: goto invalid_spec;
        }

      } break;

      // reading the pad character in the flags section "{:#X10}"
      case _fmt_state_padch: {

        if (spec.padding) {
          goto invalid_spec;
        }

        spec.padding = c;
        read_state = _fmt_state_flags;

      } break;

      // reading the prefix flag chars: "{:+<^>#_}"
      case _fmt_state_flags: {

        switch (c) {
          case '+': spec.sign = 1; break;
          case '<': spec.alignment = _fmt_align_left; break;
          case '^': spec.alignment = _fmt_align_center; break;
          case '>': spec.alignment = _fmt_align_right; break;
          case '=': spec.alignment = _fmt_align_right_leftsign; break;
          case '#': read_state = _fmt_state_padch; break;
          case '.': read_state = _fmt_state_preci; break;
          default: {
            if (!isdigit(c)) {
              goto invalid_spec;
            }
            read_state = _fmt_state_width;
          }
        }

        if (read_state != _fmt_state_width) break;

      } SWITCH_FALLTHROUGH; // fallthrough for digits

      // reading width specifier: "{:10}", "{:010}", "{:10.5}"
      case _fmt_state_width: {

        if (c == '.') {
          read_state = _fmt_state_preci;
          break;
        }

        if (!isdigit(c)) {
          goto invalid_spec;
        }

        if (c == '0' && spec.width == 0) {
          if (!spec.padding) spec.padding = '0';
          spec.alignment = _fmt_align_right_leftsign;
          break;
        }

        spec.width *= 10;
        spec.width += c - '0';

      } break;

      // precision for decimals: "{:10.5}", "{:10.5+}", "{:.4e}"
      case _fmt_state_preci: {

        if (c == '+') {
          spec.trailing = 1;
          read_state = _fmt_state_final;
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
      case _fmt_state_final: {
        goto invalid_spec;
      }

    }

  }

invalid_spec:

  spec.index = _str_fmtarg_invalid_spec;
  return spec;
}

static void _format_print_arg_align_number(
  Array_byte out, _fmt_spec_t spec, index_t excess, index_t start, index_t msd
) {
  if (!excess) return;

  switch (spec.alignment) {

    case _fmt_align_left: {
      //* pad = arr_byte_emplace_back_slice(out, excess).begin;
      //memset(pad, spec.padding, excess);
      span_byte_t pad = arr_byte_emplace_back_range(out, excess);
      span_byte_set_bytes(pad, spec.padding);
    } break;

    case _fmt_align_center: {
      index_t half = (excess + 1) / 2;
      index_t back_size = excess - half;
      span_byte_t pad = arr_byte_emplace_range(out, start, half);
      span_byte_set_bytes(pad, spec.padding);
      pad = arr_byte_emplace_back_range(out, back_size);
      span_byte_set_bytes(pad, spec.padding);
    } break;

    case _fmt_align_right: {
      span_byte_t pad = arr_byte_emplace_range(out, start, excess);
      span_byte_set_bytes(pad, spec.padding);
    } break;

    case _fmt_align_right_leftsign: {
      span_byte_t pad = arr_byte_emplace_range(out, msd, excess);
      span_byte_set_bytes(pad, spec.padding);
    } break;

  }

}

static void _format_print_arg_int(
  Array_byte out, _fmt_spec_t spec, index_t i
) {

  switch (spec.representation) {

    case _fmt_rep_char: {
      arr_byte_push_back(out,
        (i <= 0x1F || i == 0x7F || i > 127) ? '.' : (byte)i
      );
    } break;

    case _fmt_rep_default: {
      do {
        index_t digit = i % 10;
        arr_byte_push_back(out, (byte)(digit + '0'));
        i /= 10;
      } while (i);
    } break;

    case _fmt_rep_hex:
    case _fmt_rep_HEX: {
      do {
        index_t digit = i % 16;
        byte c = spec.representation == _fmt_rep_HEX ? 'A' : 'a';
        byte b = (byte)digit + (digit >= 10 ? c-10 : '0');
        arr_byte_push_back(out, b);
        i /= 16;
      } while (i);
    } break;

    case _fmt_rep_binary: {
      do {
        index_t digit = i % 2;
        arr_byte_push_back(out, (byte)(digit + '0'));
        i /= 2;
      } while (i);
    } break;

  }

}

static void _format_print_arg_vector_float(
  Array_byte out, _fmt_spec_t spec, const float* floats, index_t count
) {
  arr_byte_push_back(out, '<');
  for (index_t i = 0; i < count;) {
    int prec = spec.trailing ? -spec.precision : spec.precision;
    arr_byte_append_float(out, floats[i], prec);
    if (++i < count) {
      iarr_byte_append(out, S(", "));
    }
  }
  arr_byte_push_back(out, '>');
}

static void _format_print_arg_vector_int(
  Array_byte out, const int* ints, index_t count
) {
  arr_byte_push_back(out, '<');
  for (index_t i = 0; i < count;) {
    arr_byte_append_int(out, ints[i]);
    if (++i < count) {
      iarr_byte_append(out, S(", "));
    }
  }
  arr_byte_push_back(out, '>');
}

static void _format_print_arg(
  Array_byte out, _fmt_spec_t spec, _str_arg_t args[], index_t arg_count
) {

  if (spec.index >= arg_count) {

    // special case for padding an out-of-bounds argument
    if (spec.width) {
      span_byte_t bytes = arr_byte_emplace_back_range(out, spec.width);
      span_byte_set_bytes(bytes, spec.padding);
    }

    return;
  }

  _str_arg_t* arg = args + spec.index;

  switch (arg->type) {

    case _str_arg_slice: {

      index_t width = MAX(spec.width, arg->slice.size);
      index_t excess = width - arg->slice.size;

      byte* bytes = arr_byte_emplace_back_range(out, width).begin;

      index_t pos = 0;
      if (excess) {

        switch (spec.alignment) {

          case _fmt_align_left: {
            memset(bytes + arg->slice.size, spec.padding, excess);
          } break;

          case _fmt_align_center: {
            pos = (excess + 1) / 2;
            index_t back_size = width - arg->slice.size - pos;
            memset(bytes, spec.padding, pos);
            memset(bytes + pos + arg->slice.size, spec.padding, back_size);
          } break;

          case _fmt_align_right:
          case _fmt_align_right_leftsign: {
            pos = excess;
            memset(bytes, spec.padding, excess);
          } break;

        }

      }

      memcpy(bytes + pos, arg->slice.begin, arg->slice.size);

    } break;

    case _str_arg_int: {

      ptrdiff_t i = arg->i;
      index_t start = out->size;

      if (i < 0) {
        arr_byte_push_back(out, '-');
        i *= -1;
      } else if (spec.sign) {
        arr_byte_push_back(out, '+');
      }

      index_t msd = out->size;

      _format_print_arg_int(out, spec, i);

      span_byte_t digits = span_byte_build(out->begin + msd, out->end);
      span_byte_reverse(digits);

      index_t width = MAX(spec.width, out->size - start);
      index_t excess = width - (out->size - start);

      _format_print_arg_align_number(out, spec, excess, start, msd);

    } break;

    case _str_arg_float: {

      double f = arg->f;
      index_t start = out->size;

      if (f < 0) {
        arr_byte_push_back(out, '-');
        f *= -1;
      } else if (spec.sign) {
        arr_byte_push_back(out, '+');
      }

      // msd = most significant digit (to avoid flipping signs)
      index_t msd = out->size;

      arr_byte_append_float(out, f, spec.precision * (spec.trailing ? -1 : 1));

      index_t width = MAX(spec.width, out->size - start);
      index_t excess = width - (out->size - start);

      _format_print_arg_align_number(out, spec, excess, start, msd);

    } break;

    case _str_arg_vec2: {
      vec2 v = *((vec2*)arg->other);
      _format_print_arg_vector_float(out, spec, v.f, 2);
    } break;

    case _str_arg_vec3: {
      vec3 v = *((vec3*)arg->other);
      _format_print_arg_vector_float(out, spec, v.f, 3);
    } break;

    case _str_arg_vec4: {
      vec4 v = *((vec4*)arg->other);
      _format_print_arg_vector_float(out, spec, v.f, 4);
    } break;

    case _str_arg_vec2i: {
      vec2i v = *((vec2i*)arg->other);
      _format_print_arg_vector_int(out, v.i, 2);
    } break;

    case _str_arg_vec3i: {
      vec3i v = *((vec3i*)arg->other);
      _format_print_arg_vector_int(out, v.i, 3);
    } break;

    default: {
      byte* bytes = arr_byte_emplace_back_range(out, 22).begin;
      memcpy(bytes, "!<can't resolve type>!", 22);
    } break;

  } 

}

void arr_byte_append_format(
  Array_byte output, slice_t fmt, _str_arg_t args[], index_t argc
) {
  // Process the format string
  byte arg_index = 0;

  // track position in each section between formatters
  slice_t section = { .begin = fmt.begin, .size = 0 };

  for (index_t i = 0; i < fmt.size; ++i) {
    byte c = (byte)fmt.begin[i];

    // just do a 1:1 copy by character until we hit a format specifier
    if (c != '{') {
      ++section.size;
      continue;
    }

    // at the start of a format section, copy all the bytes up to this point
    if (section.size) {

      span_byte_t bytes = arr_byte_emplace_back_range(output, section.size);
      memcpy(bytes.begin, section.begin, section.size);
    }

    // handle case for "{{" to print escaped left brace
    if (fmt.begin[i + 1] == '{') {
      section.begin = fmt.begin + ++i;
      section.size = 1;
      continue;
    }

    index_t spec_end;
    slice_t spec_str = slice_drop(fmt, i + 1);
    _fmt_spec_t spec = _format_read_spec(spec_str, arg_index, &spec_end);

    // rather than error on invalid spec, just print the characters
    // this means we don't need to worry about escaping braces most of the time
    if (spec.index == _str_fmtarg_invalid_spec) {
      section.begin = fmt.begin + i;
      section.size = 1;
      continue;
    }

    arg_index = spec.index + 1;

    _format_print_arg(output, spec, args, argc);

    i += spec_end;
    section.begin = fmt.begin + i + 1;
    section.size = 0;
  }

  // if we reach the end and we were reading chars for output, print them here
  if (section.size) {
    span_byte_t bytes = arr_byte_emplace_back_range(output, section.size);
    memcpy(bytes.begin, section.begin, section.size);
  }
}

String istr_format(slice_t fmt, _str_arg_t args[], index_t arg_count) {
  index_t reserve_size = sizeof(String_Internal) + fmt.size;

  for (index_t i = 0; i < arg_count; ++i) {
    reserve_size += (args[i].type == _str_arg_slice) ? args[i].slice.size : 3;
  }

  // Set the starting allocation for the new string
  array_byte_t output_header = arr_byte_build_reserve(reserve_size);
  Array_byte output = &output_header;

  // Push the String header to the front of the array data
  arr_byte_emplace_back_range(output, sizeof(slice_t));

  // process the format string
  arr_byte_append_format(output, fmt, args, arg_count);

  // include the null-terminator
  arr_byte_push_back(output, '\0');
  arr_byte_truncate(output, output->size);

  // update the string's header and return
  String_Internal* header = (String_Internal*)output->begin;
  header->size = output->size - sizeof(struct slice_t) - 1;
  header->begin = header->head;
  return (String)header;
}

void istr_print(slice_t fmt, _str_arg_t args[], index_t arg_count) {
  String to_print = istr_format(fmt, args, arg_count);
  if (to_print == NULL) return;
  slice_write(to_print->slice);
  str_delete(&to_print);
}

void istr_log(slice_t fmt, _str_arg_t args[], index_t arg_count) {
  String to_print = istr_format(fmt, args, arg_count);
  if (to_print == NULL) return;
  slice_write(to_print->slice);
  str_delete(&to_print);
}

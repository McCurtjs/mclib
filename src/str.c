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

#include "str.h"
#include "utility.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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

const String str_empty      = (String)&slice_empty;
const String str_true       = (String)&slice_true;
const String str_false      = (String)&slice_false;
const String str_whitespace = (String)&slice_whitespace;
const String str_space      = (String)&slice_space;
const String str_newline    = (String)&slice_newline;
const String str_tab        = (String)&slice_tab;

#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
#endif

static String_Internal* _istr_new(index_t length) {
  if (length <= 0) return NULL; // prompt callers to return empty string
  // Include an extra byte for the null terminator
  // null isn't necessary for the slice_t interface, but it's included for
  // compatibility just in case.
  String_Internal* ret = malloc(sizeof(slice_t) + length + 1);
  assert(ret);
  ret->begin = ret->head;
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
  return str == str_empty || str == str_true || str == str_false;
}

////////////////////////////////////////////////////////////////////////////////
// Direct string str_ functions
////////////////////////////////////////////////////////////////////////////////

#include "array_byte.h"

String str_build(const char* c_str, index_t length) {
  if (!c_str) return str_empty;
  String_Internal* ret = _istr_new(length);
  if (!ret) return str_empty;
  memcpy(&ret->head, c_str, length);
  return str_terminate(ret);
}

String str_from_bool(bool b) {
  return b == FALSE ? str_false : str_true;
}

String str_from_int(int i) {
  array_byte_t arr = arr_byte_build_reserve(sizeof(String_Internal) + 10);
  arr_byte_emplace_back_range(&arr, sizeof(slice_t));
  span_byte_t number = arr_byte_append_int(&arr, i);
  arr_byte_push_back(&arr, '\0');
  arr_byte_truncate(&arr, arr.size);
  String_Internal* header = (String_Internal*)arr.begin;
  header->begin = header->head;
  header->size = span_size_bytes(number.base);
  return (String)header;
}

String str_from_float(float f) {
  array_byte_t arr = arr_byte_build_reserve(sizeof(String_Internal) + 10);
  arr_byte_emplace_back_range(&arr, sizeof(slice_t));
  span_byte_t number = arr_byte_append_float(&arr, f, 3);
  arr_byte_push_back(&arr, '\0');
  arr_byte_truncate(&arr, arr.size);
  String_Internal* header = (String_Internal*)arr.begin;
  header->begin = header->head;
  header->size = span_size_bytes(number.base);
  return (String)header;
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
  String_Internal* ret = _istr_new(str.size);
  if (!ret) return str_empty;
  memcpy(ret->begin, str.begin, str.size);
  return str_terminate(ret);
}

static index_t _istr_args_length(_str_arg_t args[], index_t argc) {
  index_t length = 0;

  // accumulate length of final combined string
  for (index_t i = 0; i < argc; ++i) {
    switch (args[i].type) {

      case _str_arg_slice: {
        length += args[i].slice.size;
      } break;

      case _str_arg_span: {
        span_slice_t span = args[i].span;
        for (slice_t* slice = span.begin; slice != span.end; ++slice) {
          length += slice->size;
        }
      } break;

      case _str_arg_int: {
        length += 1;
      } break;

      default: {
        assert(false);
      } break;

    }
  }

  return length;
}

static index_t _istr_args_count(_str_arg_t args[], index_t argc) {
  index_t count = 0;

  // accumulate length of final combined string
  for (index_t i = 0; i < argc; ++i) {
    switch (args[i].type) {

      case _str_arg_slice:
      case _str_arg_int:
        ++count;
        break;

      case _str_arg_span:
        count += span_slice_size(args[i].span);
        break;

      default:
        assert(false);
        break;

    }
  }

  return count;
}

static char* _istr_arg_write(char* dst, _str_arg_t* arg) {
  switch (arg->type) {

    case _str_arg_slice:
      memcpy(dst, arg->slice.begin, arg->slice.size);
      return dst + arg->slice.size;

    case _str_arg_span:
      span_slice_t span = arg->span;
      for (slice_t* slice = span.begin; slice != span.end; ++slice) {
        memcpy(dst, slice->begin, slice->size);
        return dst + slice->size;
      }
      return dst;

    case _str_arg_int:
      *dst = (char)arg->i;
      return dst + 1;

    default:
      assert(false);
      break;

  }

  return dst;
}

String istr_concat(_str_arg_t args[], index_t count) {
  if (count <= 0) return str_empty;

  index_t length = _istr_args_length(args, count);
  String_Internal* ret = _istr_new(length);
  if (!ret) return str_empty;

  char* dst = ret->begin;
  for (index_t i = 0; i < count; ++i) {
    dst = _istr_arg_write(dst, &args[i]);
  }

  return str_terminate(ret);
}

String istr_join(slice_t del, _str_arg_t args[], index_t count) {
  if (count <= 0) return str_empty;
  if (del.size < 0) del = slice_empty;

  index_t full_count = _istr_args_count(args, count);
  if (full_count <= 0) return str_empty;
  index_t length = _istr_args_length(args, count);
  length += del.size * (full_count - 1);

  String_Internal* ret = _istr_new(length);
  if (!ret) return str_empty;

  char* dst = ret->begin;

  for (index_t i = 0; i < count; ++i) {
    if (args[i].type != _str_arg_span) {
      dst = _istr_arg_write(dst, &args[i]);

      if (i != count - 1) {
        memcpy(dst, del.begin, del.size);
        dst += del.size;
      }
    } else { // args[i].type == _str_arg_span
      span_slice_t span = args[i].span;

      for (index_t j = 0; j < span_slice_size(span); ++j) {
        memcpy(dst, span.begin[j].begin, span.begin[j].size);
        dst += span.begin[j].size;

        if (i != count - 1 || j != span_slice_size(span) - 1) {
          memcpy(dst, del.begin, del.size);
          dst += del.size;
        }
      }
    }
  }

  return str_terminate(ret);
}

Array_slice istr_split(slice_t str, _str_arg_t args[], index_t count) {
  Array_slice ret = arr_slice_new_reserve(_istr_args_count(args, count) * 2);

  if (count <= 0) {
    arr_slice_push_back(ret, str);
    arr_slice_truncate(ret, ret->size);
    return ret;
  }
  
  index_t pos = 0;
  while (pos < str.size) {
    index_t index = str.size;
    slice_t* slice = arr_slice_emplace_back(ret);

    for (index_t i = 0; i < count; ++i) {

      index_t check = pos;
      slice_t next;

      switch (args[i].type) {
        case _str_arg_int:
          slice_t charslice = slice_build((char*)&args[i].i, 1);
          next = slice_token_char(str, charslice, &check).token;
          break;

        case _str_arg_slice:
          next = slice_token_str(str, args[i].slice, &check).token;
          break;

        case _str_arg_span:
          next = slice_token_any(str, args[i].span, &check).token;
          break;

        default:
          assert(false);
          next = slice_empty;
          break;
      }

      if (check < index) {
        *slice = next;
        index = check;
      }
    }

    if (index == str.size) {
      *slice = slice_drop(str, pos);
    }

    pos = index;
  }

  return ret;
}

Array_slice istr_tokenize(
  slice_t str, _str_arg_t args[], index_t count
) {
  Array_slice ret = arr_slice_new_reserve(_istr_args_count(args, count) * 2);

  if (count <= 0) {
    arr_slice_push_back(ret, str);
    arr_slice_truncate(ret, ret->size);
    return ret;
  }

  index_t pos = 0;
  while (pos < str.size) {
    index_t index = str.size;
    slice_t* slice = arr_slice_emplace_back(ret);

    for (index_t i = 0; i < count; ++i) {

      index_t check = pos;
      slice_t next;

      switch (args[i].type) {
        case _str_arg_int:
          slice_t charslice = slice_build((char*)&args[i].i, 1);
          next = slice_token_char(str, charslice, &check).token;
          break;

        case _str_arg_slice:
          next = slice_token_str(str, args[i].slice, &check).token;
          break;

        case _str_arg_span:
          next = slice_token_any(str, args[i].span, &check).token;
          break;

        default:
          assert(false);
          next = slice_empty;
          break;
      }

      if (check < index) {
        *slice = next;
        index = check;
      }
    }

    if (index == str.size) {
      *slice = slice_drop(str, pos);
    }

    pos = index;
  }

  return ret;
}

String istr_prepend(slice_t str, index_t length, char c) {
  String_Internal* ret = _istr_new(str.size + length);
  if (!ret) return str_empty;
  memset(ret->begin, c, length);
  memcpy(ret->begin + length, str.begin, str.size);
  return str_terminate(ret);
}

String istr_append(slice_t str, index_t length, char c) {
  String_Internal* ret = _istr_new(str.size + length);
  if (!ret) return str_empty;
  memcpy(ret->begin, str.begin, str.size);
  memset(ret->begin + str.size, c, length);
  return str_terminate(ret);
}

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

#include "slice.h"

#include <string.h> // strlen, memcmp
#include <ctype.h> // tolower, isdigit, isspace

const char slice_constants[] = "\0true\0false";

const slice_t slice_empty = { .begin = &slice_constants[0], .size = 0 };
const slice_t slice_true = { .begin = &slice_constants[1], .size = 4 };
const slice_t slice_false = { .begin = &slice_constants[6], .size = 5 };

#define SLICE_VALID(str)                                                      \
  assert(str.size >= 0);                                                      \
  assert(str.size > 0 ? str.begin != NULL : true)                             //

slice_t slice_build(const char* c_str) {
  if (c_str == NULL) return slice_empty;
  return (slice_t) {
    .begin = c_str,
    .length = strlen(c_str),
  };
}

bool slice_eq(slice_t lhs, slice_t rhs) {
  SLICE_VALID(lhs);
  SLICE_VALID(rhs);
  if (lhs.size != rhs.size) return FALSE;
  return memcmp(lhs.begin, rhs.begin, lhs.size) == 0;
}

bool slice_starts_with(slice_t str, slice_t starts) {
  SLICE_VALID(str);
  SLICE_VALID(starts);
  if (starts.size > str.size) return FALSE;
  return memcmp(str.begin, starts.begin, starts.size) == 0;
}

bool slice_ends_with(slice_t str, slice_t ends) {
  SLICE_VALID(str);
  SLICE_VALID(ends);
  if (ends.size > str.size) return FALSE;
  return memcmp(str.begin + str.size - ends.size, ends.begin, ends.size) == 0;
}

bool slice_contains(slice_t str, slice_t check) {
  return slice_find(str, check, NULL);
}

bool slice_contains_char(slice_t str, char check) {
  SLICE_VALID(str);
  for (index_t i = 0; i < str.size; ++i) {
    if (str.begin[i] == check) {
      return true;
    }
  }
  return false;
}

bool slice_is_empty(slice_t str) {
  SLICE_VALID(str);
  if (str.size == 0) return true;
  for (index_t i = 0; i < str.size; ++i) {
    if (!isspace(str.begin[i])) {
      return false;
    }
  }
  return true;
}

bool slice_to_bool(slice_t str, bool* out) {
  SLICE_VALID(str);
  assert(out);
  if (str.size < 4) return false;
  if (tolower(str.begin[0]) == 't') {
    for (index_t i = 1; i < 4; ++i) {
      if (tolower(str.begin[i]) != slice_true.begin[i]) return false;
    }
    *out = true;
    return true;
  }
  else if (tolower(str.begin[0]) == 'f') {
    for (index_t i = 1; i < 5; ++i) {
      if (tolower(str.begin[i]) != slice_false.begin[i]) return false;
    }
    *out = false;
    return true;
  }
  return false;
}

bool slice_to_int(slice_t str, int* out) {
  SLICE_VALID(str);
  assert(out);
  index_t long_out;
  bool ret = slice_to_long(str, &long_out);
  if (ret) *out = (int)long_out;
  return ret;
}

bool slice_to_long(slice_t str, index_t* out) {
  SLICE_VALID(str);
  assert(out);
  if (!out || str.size == 0) return false;
  index_t sign = 1;
  index_t start = 0;

  if (str.begin[0] == '-') {
    sign = -1;
    ++start;
  }
  else if (str.begin[0] == '+') {
    ++start;
  }

  index_t num = 0;
  index_t i = start;
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

bool slice_to_float(slice_t str, float* out) {
  SLICE_VALID(str);
  assert(out);
  double long_out;
  bool ret = slice_to_double(str, &long_out);
  if (ret) *out = (float)long_out;
  return ret;
}

bool slice_to_double(slice_t str, double* out) {
  SLICE_VALID(str);
  assert(out);

  index_t i = 0;
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
  }
  else if (s[i] == '+') {
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

bool slice_find(slice_t str, slice_t to_find, slice_t* out_opt_slice) {
  SLICE_VALID(str);
  SLICE_VALID(to_find);
  index_t index = slice_index_of(str, to_find, 0);
  if (index >= str.length) return false;
  if (!out_opt_slice) return true;
  out_opt_slice->begin = str.begin + index;
  out_opt_slice->size = str.size;
  return true;
}

index_t slice_index_of_char(slice_t str, char c, index_t from_pos) {
  SLICE_VALID(str);
  if (from_pos >= str.size) return str.size;
  if (from_pos < 0) from_pos += str.size;
  if (from_pos < 0) from_pos = 0;
  for (index_t i = from_pos; i < str.size; ++i) {
    if (str.begin[i] == c) {
      return i;
    }
  }
  return str.size;
}

index_t slice_index_of(slice_t str, slice_t to_find, index_t from_pos) {
  SLICE_VALID(str);
  SLICE_VALID(to_find);
  if (str.size < to_find.size) return str.size;
  if (from_pos >= str.size) return str.size;
  if (from_pos < 0) from_pos += str.size;
  if (from_pos < 0) from_pos = 0;
  if (to_find.size == 0) return MIN(from_pos, str.size);
  index_t j;
  for (index_t i = from_pos; i <= str.size - to_find.size; ++i) {
    j = 0;
    while (j < to_find.size) {
      if (str.begin[i + j] != to_find.begin[j]) break;
      if (++j == to_find.size) return i;
    }
  }
  return str.size;
}

slice_t slice_token(slice_t str, slice_t to_find, index_t* pos) {
  assert(pos != NULL);
  assert(to_find.size != 0);
  if (str.size <= *pos) return slice_empty;

  for (index_t i = *pos; i < str.size; ++i) {
    for (index_t d = 0; d < to_find.size; ++d) {
      if (to_find.begin[d] == str.begin[i]) {
        index_t old_pos = *pos;
        *pos = i + 1;
        return (slice_t) {
          .begin = str.begin + old_pos,
          .size = i - old_pos
        };
      }
    }
  }

  *pos = str.size;
  return slice_empty;
}

slice_t islice_substring(slice_t str, index_t start, index_t end) {
  SLICE_VALID(str);
  if (start == end) return slice_empty;
  if (start >= str.size) return slice_empty;
  if (start < 0) start += str.size;
  if (start < 0) start = 0;
  if (end > str.size) end = str.size;
  if (end < 0) end = str.size + end;
  if (end <= start) return slice_empty;
  return (slice_t) {
    .begin = str.begin + start,
    .size = end - start,
  };
}

slice_t slice_trim(slice_t str) {
  slice_t ret = slice_trim_start(str);
  return slice_trim_end(ret);
}

slice_t slice_trim_start(slice_t str) {
  SLICE_VALID(str);
  index_t start, end = str.size;
  for (start = 0; start < str.size; ++start) {
    if (!isspace(str.begin[start])) break;
  }
  if (start == end) return slice_empty;
  return (slice_t) {
    .begin = str.begin + start,
    .size = end - start,
  };
}

slice_t slice_trim_end(slice_t str) {
  SLICE_VALID(str);
  index_t end = str.size;
  while (end > 0) {
    if (isspace(str.begin[end - 1])) --end;
    else break;
  }
  if (end <= 0) return slice_empty;
  return (slice_t) {
    .begin = str.begin,
    .size = end,
  };
}

/*
span_byte_t slice_to_span(slice_t slice) {
  return (span_byte_t) { .begin = slice.begin, .end = slice.begin + slice.size };
}*/

slice_t span_byte_to_slice(span_byte_t span) {
  assert(span.end >= span.begin);
  slice_t ret = { .begin = (char*)span.begin, .size = (span.end - span.begin) };
  if (span.end > span.begin && *(span.end - 1) == '\0') {
    ret.size -= 1;
  }
  return ret;
}

Array_slice slice_split(slice_t str, slice_t del) {
  SLICE_VALID(str);
  SLICE_VALID(del);
  Array_slice ret = arr_slice_new();

  // specialization for empty delimiter, return a slice for each char
  if (del.size == 0) {
    arr_slice_reserve(ret, str.size);
    for (index_t i = 0; i < str.size; ++i) {
      slice_t c = slice_build_s(&str.begin[i], 1);
      arr_slice_push_back(ret, c);
    }
    return ret;
  }

  index_t i = 0;
  do {
    index_t prev = i;
    i = slice_index_of(str, del, i);
    slice_t slice = slice_substring(str, prev, i);
    arr_slice_push_back(ret, slice);
    i += del.size;
    if (i == str.size) arr_slice_push_back(ret, slice_empty);
  } while (i < str.size);

  return ret;
}

#include <stdio.h>
void slice_write(slice_t str) {
  SLICE_VALID(str);
  uint length = (uint)str.size;
  printf("%.*s\n", length, str.begin);
  fflush(stdout); // SDL Window blocks console output for some reason
}

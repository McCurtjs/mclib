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

#include "slice.h"

#include "array_slice.h"

#include "utility.h" // hash

#include <string.h> // strlen, memcmp
#include <ctype.h> // tolower, isdigit, isspace

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
# pragma warning ( disable : 28182 )
#endif

const char slice_constants[] = "\0true\0false";

const slice_t slice_empty = { .begin = &slice_constants[0], .size = 0 };
const slice_t slice_true = { .begin = &slice_constants[1], .size = 4 };
const slice_t slice_false = { .begin = &slice_constants[6], .size = 5 };

#define SLICE_VALID(str)                                                      \
  assert((str).size >= 0);                                                    \
  assert((str).size > 0 ? (str).begin != NULL : true)                         //

// Builds a slice from a null-terminated c-style string.
slice_t slice_from_c_str(const char* c_str) {
  if (c_str == NULL) return slice_empty;
  return (slice_t) {
    .begin = c_str,
    .length = strlen(c_str),
  };
}

////////////////////////////////////////////////////////////////////////////////
// Parsing for basic types
////////////////////////////////////////////////////////////////////////////////

// Parses the string into a boolean value. Parsing is not case sensitive.
//
// \returns whether or not the parse is successful, not the value of the parsed
//    boolean. The parse result is written to the `out` parameter on success.
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

// Parses the string slice into a signed integer value.
bool slice_to_int(slice_t str, int* out) {
  SLICE_VALID(str);
  assert(out);
  index_t long_out;
  bool ret = slice_to_long(str, &long_out);
  if (ret) *out = (int)long_out;
  return ret;
}

// Parses the string slice into a signed integer value.
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

// Parses the string slice into a single-precision floating point value.
bool slice_to_float(slice_t str, float* out) {
  SLICE_VALID(str);
  assert(out);
  double long_out;
  bool ret = slice_to_double(str, &long_out);
  if (ret) *out = (float)long_out;
  return ret;
}

// Parses the string slice into a double-precision floating point value.
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

////////////////////////////////////////////////////////////////////////////////
// Comparisons
////////////////////////////////////////////////////////////////////////////////

// Compares two slices returning 0 if equivalent, or -1 or 1 if the left slice
// is lexicographically less than or greater than the right slice.
int slice_compare(slice_t lhs, slice_t rhs) {
  SLICE_VALID(lhs);
  SLICE_VALID(rhs);
  if (lhs.size != rhs.size) return (int)(rhs.size - lhs.size);
  return memcmp(lhs.begin, rhs.begin, lhs.size);
}

// True if the slice is lexicographically equivalent to the other string.
// Comparison is case-sensitive.
bool slice_eq(slice_t lhs, slice_t rhs) {
  SLICE_VALID(lhs);
  SLICE_VALID(rhs);
  if (lhs.size != rhs.size) return FALSE;
  return memcmp(lhs.begin, rhs.begin, lhs.size) == 0;
}

// True if the slice starts with the given string, false otherwise.
bool slice_starts_with(slice_t str, slice_t starts) {
  SLICE_VALID(str);
  SLICE_VALID(starts);
  if (starts.size > str.size) return FALSE;
  return memcmp(str.begin, starts.begin, starts.size) == 0;
}

// True if the slice ends with the given string, false otherwise.
bool slice_ends_with(slice_t str, slice_t ends) {
  SLICE_VALID(str);
  SLICE_VALID(ends);
  if (ends.size > str.size) return FALSE;
  return memcmp(str.begin + str.size - ends.size, ends.begin, ends.size) == 0;
}

// True if the slice contains the given target string, false otherwise.
bool slice_contains(slice_t str, slice_t target) {
  return slice_find_str(str, target, NULL);
}

// True if the slice contains at least one of the target characters.
bool slice_contains_char(slice_t str, slice_t targets) {
  return slice_index_of_char(str, targets) != str.size;
}

// True if the slice contains only whitespace, false otherwise.
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

////////////////////////////////////////////////////////////////////////////////
// Basic searching
////////////////////////////////////////////////////////////////////////////////

// Returns true if the string contains the target string, false otherwise.
// If the target is found, an optional output can be set to the found result.
bool slice_find_str(slice_t str, slice_t target, slice_t* out_opt_slice) {
  SLICE_VALID(str);
  SLICE_VALID(target);
  index_t index = slice_index_of_str(str, target);
  if (index == str.size) return false;
  if (out_opt_slice) {
    out_opt_slice->begin = str.begin + index;
    out_opt_slice->size = target.size;
  }
  return true;
}

// Returns true if the string contains the target string, false otherwise.
// If the target is found, an optional output can be set to the found result.
bool slice_find_last_str(slice_t str, slice_t target, slice_t* out_opt_slice) {
  SLICE_VALID(str);
  SLICE_VALID(target);
  index_t index = slice_index_of_last_str(str, target);
  if (!out_opt_slice) return index != str.size;
  out_opt_slice->begin = str.begin + index;
  out_opt_slice->size = target.size;
  return index != str.size;
}

// Gets the index of the search string in `str`.
//
// \returns the index of the substring, or str.size if not found.
index_t slice_index_of_str(slice_t str, slice_t target) {
  SLICE_VALID(str);
  SLICE_VALID(target);
  assert(target.size > 0);
  if (str.size < target.size) return str.size;
  index_t sublength = str.size - target.size;
  for (index_t i = 0; i <= sublength; ++i) {
    for (index_t j = 0; j < target.size;) {
      if (str.begin[i + j] != target.begin[j]) break;
      if (++j >= target.size) return i;
    }
  }
  return str.size;
}

// Gets the index of the last instance of the target in the string.
//
// \returns the index of the substring, or str.size if not found.
index_t slice_index_of_last_str(slice_t str, slice_t target) {
  SLICE_VALID(str);
  SLICE_VALID(target);
  assert(target.size > 0);
  if (str.size < target.size) return str.size;
  for (index_t i = str.size - target.size; i >= 0; --i) {
    index_t j = 0;
    while (j < target.size) {
      if (str.begin[i + j] != target.begin[j]) break;
      if (++j >= target.size) return i;
    }
  }
  return str.size;
}

// Gets the index of any of the given charactesr in the targets slice.
//
// \returns the index of the character, or str.size if not found.
index_t slice_index_of_char(slice_t str, slice_t targets) {
  SLICE_VALID(str);
  SLICE_VALID(targets);
  assert(targets.size > 0);
  for (index_t i = 0; i < str.size; ++i) {
    for (index_t j = 0; j < targets.size; ++j) {
      if (str.begin[i] == targets.begin[j]) {
        return i;
      }
    }
  }
  return str.size;
}

// Gets the index of any of the given charactesr in the targets slice.
//
// \returns the index of the character, or str.size if not found.
index_t slice_index_of_last_char(slice_t str, slice_t targets) {
  SLICE_VALID(str);
  SLICE_VALID(targets);
  assert(targets.size > 0);
  for (index_t i = str.size - 1; i >= 0; --i) {
    for (index_t j = 0; j < targets.size; ++j) {
      if (str.begin[i] == targets.begin[j]) {
        return i;
      }
    }
  }
  return str.size;
}

////////////////////////////////////////////////////////////////////////////////
// Tokenization
////////////////////////////////////////////////////////////////////////////////

#define _token_result_empty (token_result_t) { slice_empty, slice_empty }

// Get the next slice in the string that terminates with the given delimiter,
//    starting at the provided position `pos`.
//
// \param delim - string of possible characters that appear between tokens.
//
// \param pos - pointer to an index representing the starting position of the
//              token. After running, this value is changed to one past the next
//              delimiter, and can be re-passed in with the same underlying
//              string to continue reading the next token.
token_result_t slice_token_str(slice_t str, slice_t delim, index_t* pos) {
  SLICE_VALID(str);
  SLICE_VALID(delim);
  assert(pos != NULL);
  assert(delim.size > 0);
  if (str.size <= *pos) return _token_result_empty;

  token_result_t ret = (token_result_t) {
    .token = slice_substring(str, *pos),
    .delimiter = slice_empty,
  };

  if (slice_find_str(ret.token, delim, &ret.delimiter)) {
    ret.token.size = ret.delimiter.begin - ret.token.begin;
  }

  *pos += ret.token.size + ret.delimiter.size;
  return ret;
}

// Get the next slice in the string that terminates with any one of the
//    characters in the delimiters string, starting at the position of `pos`.
//
// \param delims - string of possible characters that appear between tokens.
//
// \param pos - pointer to index representing starting position of the token.
//              Value is updated to the start of the next token.
token_result_t slice_token_char(slice_t str, slice_t delims, index_t* pos) {
  SLICE_VALID(str);
  SLICE_VALID(delims);
  assert(pos != NULL);
  assert(delims.size > 0);
  if (str.size <= *pos) return _token_result_empty;

  token_result_t ret = (token_result_t) {
    .token = slice_substring(str, *pos),
    .delimiter = slice_empty,
  };

  index_t index = slice_index_of_char(ret.token, delims);

  if (index >= ret.token.size) {
    *pos = str.size;
    return ret;
  }

  ret.token.size = index;
  ret.delimiter.begin = ret.token.begin + index;
  ret.delimiter.size = 1;

  *pos += index + 1;
  return ret;
}

token_result_t slice_token_any(slice_t str, span_slice_t any, index_t* pos) {
  SLICE_VALID(str);
  assert(pos);
  index_t span_size = span_slice_size(any);
  if (str.size <= *pos) return _token_result_empty;

  token_result_t ret = (token_result_t) {
    .token = slice_substring(str, *pos),
    .delimiter = slice_empty,
  };

  slice_t check = ret.token;
  while (check.size) {
    for (index_t d = 0; d < span_size; ++d) {
      slice_t delim = span_slice_get(any, d);
      if (delim.size <= 0 || delim.size > check.size) continue;

      // Track the string manually rather than using slice_starts_with so we can
      //    benefit from the early-out for each delimiter and ensure we match
      //    the delimiters in the order they were given.
      for (index_t i = 0; check.begin[i] == delim.begin[i];) {
        if (++i >= delim.size) {
          ret.token.size = check.begin - ret.token.begin;
          ret.delimiter.begin = check.begin;
          ret.delimiter.size = i;
          *pos += ret.token.size + i;
          return ret;
        }
      }
    }

    ++check.begin;
    --check.size;
  }

  *pos = str.size;
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Split
////////////////////////////////////////////////////////////////////////////////

// Specialization for empty delimiter, return a slice for each char
Array_slice _slice_split_all_chars(slice_t str) {
  Array_slice ret = arr_slice_new_reserve(str.size);
  for (index_t i = 0; i < str.size; ++i) {
    slice_t* slice = arr_slice_emplace_back(ret);
    *slice = slice_build(&str.begin[i], 1);
  }
  return ret;
}

// Splits a string slice into an array of slices based on a delimiter.
Array_slice slice_split_str(slice_t str, slice_t delim) {
  SLICE_VALID(str);
  SLICE_VALID(delim);
  if (delim.size <= 0) return _slice_split_all_chars(str);
  Array_slice ret = arr_slice_new();

  index_t i = 0;
  do {
    slice_t slice = slice_token_str(str, delim, &i).token;
    arr_slice_push_back(ret, slice);
  } while (i < str.size);

  arr_slice_truncate(ret, 0);
  return ret;
}

// Splits a string slice into an array of slices based on a delimiter.
Array_slice slice_split_str_with_delim(slice_t str, slice_t delim) {
  SLICE_VALID(str);
  SLICE_VALID(delim);
  if (delim.size <= 0) return _slice_split_all_chars(str);
  Array_slice ret = arr_slice_new();

  index_t i = 0;
  loop {
    token_result_t result = slice_token_str(str, delim, &i);
    arr_slice_push_back(ret, result.token);
    until (i >= str.size);
    arr_slice_push_back(ret, result.delimiter);
  }

  return ret;
}

// Splits a string slice into an array of slices based on a delimiter.
Array_slice slice_split_char(slice_t str, slice_t delims) {
  SLICE_VALID(str);
  SLICE_VALID(delims);
  if (delims.size <= 0) return _slice_split_all_chars(str);
  Array_slice ret = arr_slice_new();

  index_t i = 0;
  do {
    slice_t slice = slice_token_char(str, delims, &i).token;
    arr_slice_push_back(ret, slice);
  } while (i < str.size);

  return ret;
}

// Splits a string slice into an array of slices based on a delimiter.
Array_slice slice_split_char_with_delim(slice_t str, slice_t delims) {
  SLICE_VALID(str);
  SLICE_VALID(delims);
  if (delims.size <= 0) return _slice_split_all_chars(str);
  Array_slice ret = arr_slice_new();

  index_t i = 0;
  loop {
    token_result_t result = slice_token_char(str, delims, &i);
    arr_slice_push_back(ret, result.token);
    until (i >= str.size);
    arr_slice_push_back(ret, result.delimiter);
  }

  return ret;
}

// Splits a string slice into an array of slices based on a delimiter.
Array_slice slice_split_any(slice_t str, span_slice_t delims) {
  SLICE_VALID(str);
  if (span_slice_size(delims) <= 0) return _slice_split_all_chars(str);
  Array_slice ret = arr_slice_new();

  index_t i = 0;
  do {
    slice_t slice = slice_token_any(str, delims, &i).token;
    arr_slice_push_back(ret, slice);
  } while (i < str.size);

  return ret;
}

// Splits a string slice into an array of slices based on a delimiter.
Array_slice slice_split_any_with_delim(slice_t str, span_slice_t delims) {
  SLICE_VALID(str);
  if (span_slice_size(delims) <= 0) return _slice_split_all_chars(str);
  Array_slice ret = arr_slice_new();

  index_t i = 0;
  loop {
    token_result_t result = slice_token_any(str, delims, &i);
    arr_slice_push_back(ret, result.token);
    until (i >= str.size);
    arr_slice_push_back(ret, result.delimiter);
  }

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Substrings
////////////////////////////////////////////////////////////////////////////////

// Gets a substring within the slice without copying any data.
//
// If start or end position are negative, they are treated as representing that
//    many characters from the end of the string.
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

// Trims leading and trailing whitespace from the slice.
slice_t slice_trim(slice_t str) {
  slice_t ret = slice_trim_start(str);
  return slice_trim_end(ret);
}

// Trims leading whitespace from the slice.
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

// Trims trailing whitespace from the slice.
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

////////////////////////////////////////////////////////////////////////////////
// Misc.
////////////////////////////////////////////////////////////////////////////////

// Returns a hash value representing the slice.
hash_t slice_hash(slice_t str) {
  SLICE_VALID(str);
  return hash(str.begin, str.size);
}

// Compares two slices passed as void pointers.
int slice_compare_vptr(const void* lhs, const void* rhs) {
  assert(lhs && rhs);
  return slice_compare(*(slice_t*)lhs, *(slice_t*)rhs);
}

// Returns a hash value representing the slice passed as a void pointer.
hash_t slice_hash_vptr(const void* str) {
  assert(str);
  return slice_hash(*(slice_t*)str);
}

#if defined(__WASM__) || defined(MCLIB_NO_STDIO)

// No default printer for WASM or when requested to be disabled.
static void _slice_write_default(slice_t str) {
  PARAM_UNUSED(str);
}

#else

#include <stdio.h> // printf, fflush

// Default output method for printing string slices with printf.
static void _slice_write_default(slice_t str) {
  SLICE_VALID(str);
  uint length = (uint)str.size;
  printf("%.*s\n", length, str.begin);
  fflush(stdout); // SDL Window blocks console output for some reason
}

#endif

void (*slice_write)(slice_t str) = _slice_write_default;

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

#ifndef MCLIB_STRING_H_
#define MCLIB_STRING_H_

#include "types.h"

#include "array_slice.h"

// \brief String is a handle type pointing to an immutable string on the heap.
//
// \brief Unlike slice_t, a String object has ownership of the data in its
//    contained range. Any String object returned from a function will have to
//    later be freed via str_delete(). Any slice returned from a function using
//    the String's range will have its lifecycle bound to the String, and will
//    be invalid once the String object is deleted.
typedef struct _opaque_String_base {
  union {
    const slice_t slice;
    struct {
      const char* const begin;
      union {
        const index_t length;
        const index_t size;
      };
    };
  };
}* String;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

// \brief String constant for an empty string
extern const String str_empty;

// \brief String constant for a "true" string
extern const String str_true;

// \brief String constant for a "false" string
extern const String str_false;

// \brief slice constant for a string representing whitespace characters
extern const String str_whitespace;

// \brief slice constant for a " " string
extern const String str_space;

// \brief slice constant for a newline character
extern const String str_newline;

// \brief slice constant for a tab character
extern const String str_tab;

////////////////////////////////////////////////////////////////////////////////
// Helper tools for type coalescing and variadic formgat arguments
////////////////////////////////////////////////////////////////////////////////

// \brief Macro to coalesce a String, slice, or char* into a slice.
#define _s2r(str) _Generic((str), \
  slice_t:      slice_from_slice, \
  String:       slice_from_str,   \
  char*:        slice_from_c_str, \
  const char*:  slice_from_c_str  \
)(str)                            //

typedef enum _str_argtype_t {
  _str_arg_end,
  _str_arg_slice,
  _str_arg_span,
  _str_arg_int,
  _str_arg_float
} _str_argtype_t;

typedef struct _str_arg_t {
  _str_argtype_t type;
  union {
    slice_t slice;
    span_slice_t span;
    ptrdiff_t i;
    double f;
  };
} _str_arg_t;

////////////////////////////////////////////////////////////////////////////////
// Construction and basic type serialization
////////////////////////////////////////////////////////////////////////////////

#define str_copy(str)               istr_copy(_s2r(str))
String  str_build(const char* c_str, index_t length);
String  str_from_bool(bool b);
String  str_from_int(int i);
String  str_from_float(float f);

void    str_delete(String* str);

static inline slice_t slice_from_str(String s) { return s->slice; }

// \brief Returns size of a string. Equivalent to str->size, but can also be
//    applied to slices and c-strings.
#define str_size(str)               slice_size(_s2r(str))

////////////////////////////////////////////////////////////////////////////////
// Parsing and basic type comparisons
////////////////////////////////////////////////////////////////////////////////

#define str_to_bool(str, out)       slice_to_bool(_s2r(str), out)
#define str_to_int(str, out)        slice_to_int(_s2r(str), out)
#define str_to_long(str, out)       slice_to_long(_s2r(str), out)
#define str_to_float(str, out)      slice_to_float(_s2r(str), out)
#define str_to_double(str, out)     slice_to_double(_s2r(str), out)

#define str_compare(lhs, rhs)       slice_compare(_s2r(lhs), _s2r(rhs))
#define str_compare_ci(lhs, rhs)    slice_compare_ci(_s2r(lhs), _s2r(rhs))
#define str_eq(lhs, rhs)            slice_eq(_s2r(lhs), _s2r(rhs))
#define str_eq_ci(lhs, rhs)         slice_eq_ci(_s2r(lhs), _s2r(rhs))
#define str_starts_with(str, start) slice_starts_with(_s2r(str), _s2r(start))
#define str_ends_with(str, end)     slice_ends_with(_s2r(str), _s2r(end))
#define str_contains(str, check)    slice_contains(_s2r(str), _s2r(check))
#define str_contains_char(s, chs)   slice_contains_char(_s2r(s), _s2r(chs))
#define str_contains_any(s, delims) slice_contains_any(_s2r(s), delims)
bool    str_is_null_or_empty(const String str);
#define str_count_str(str, delim)   slice_count_str(_s2r(str), _s2r(delim))
#define str_count_char(str, delims) slice_count_char(_s2r(str), _s2r(delims))
#define str_count_any(str, any)     slice_count_any(_s2r(str), any)

#define str_find(str, target, out)  slice_find_str(_s2r(str), _s2r(target))

#define str_find_last(st, t, out)   slice_find_last_str(_s2r(st), _s2r(t), out))
#define str_find_last_char(s, t, o) slice_find_last_char(_s2r(s), _s2r(t), o))
#define str_find_last_any(s, a, o)  slice_find_last_any(_s2r(s), a, o))

// \brief Gets the index of the next instance of the target string.
//
// \returns the index of the match, or str.size if it's not present.
#define str_index_of(str, target)   slice_index_of_str(_s2r(str), _s2r(target))

// \brief Gets the start of the next instance of the target string.
//
// \returns the index of the match, or str.size if it's not present.
#define str_index_of_char(str, tgt) slice_index_of_char(_s2r(str), tgt)

// \brief Gets the index of the next instance of any of the target strings.
//
// \returns the index of the match, or str.size if it's not present.
#define str_index_of_any(str, tgts) slice_index_of_any(_s2r(str), tgts)

// \brief Gets a token as a substring of str described by the starting position
//    pos that ends with (not including) any delimeter character in to_find.
//
// \brief The value of pos is updated to the index after the found delimeter.
//    If no character was found in the string, the value is equal to str.size.
//
// \returns
//    A slice containing the token
#define str_token(str, delim, pos)  slice_token_str(_s2r(str), _s2r(delim), pos)

#define str_token_char(s, dels, p)  slice_token_char(_s2r(s), _s2r(dels), p)

#define str_token_any(s, any, pos)  slice_token_any(_s2r(s), any, pos)

////////////////////////////////////////////////////////////////////////////////
// Substrings
////////////////////////////////////////////////////////////////////////////////

// \brief Gets a substring as a slice within the input string or slice.
//
// \brief Works like javascript string.slice. Negative offsets for either the
//    start or end of the string represent an offset from the end of the string.
//
// \param str The string or slice to get a substring of. The returned
//    substring's lifetime will be dependent on the lifetime of str.
//
// \param start - The beginning of the subrange, inclusive.
//    - A non-negative value represents an offset from the beginning.
//    - A value less than zero represents an offset from the end.
//
// \param end - [optional] The end of the subrange, exclusive.
//    - A zero or positive value represents an offset from the beginning.
//    - A value less than one represents an offset from the end.
//
// \param __VA_ARGS__ - start, ?end
//
// \returns a slice as a substring of the input string.
#define str_substring(str, st, end) slice_substring(_s2r(str), st, end)

// \brief Alias for `str_substring`.
#define str_slice(str, st, end)     slice_substring(_s2r(str), st, end)

// \brief Drops `count` characters from the start (if positive) or end (if
//    negative) of the string.
#define str_drop(str, count)        slice_drop(_s2r(str), count)

// \brief Gets a substring of the first `count` characters of the start (if
//    positive) or end (if negative) from the string.
#define str_take(str, count)        slice_take(_s2r(str), count)

// \brief Returns a slice with leading and trailing whitespace omitted.
#define str_trim(str)               slice_trim(_s2r(str))

// \brief Returns a slice with leading whitespace omitted.
#define str_trim_start(str)         slice_trim_start(_s2r(str))

// \brief Returns a slice with trailing whitespace omitted.
#define str_trim_end(str)           slice_trim_end(_s2r(str))

////////////////////////////////////////////////////////////////////////////////
// Splits and joins
////////////////////////////////////////////////////////////////////////////////

// \brief Splits the string into an array of substrings based on the delimiter.
//
// \param str - The string to split into pieces.
//
// \param delims - delimiters to split the string against. Allowed params are
//    string types (String, slice_t, char*) or single chars ('x'). The
//    delimiters themselves are not included in the output.
//
// \returns An array of slices whose lifetimes are bound to str.
//    The Array must be deleted by the user via arr_str_delete(&arr).
#define str_split(str, ...) istr_split(_s2r(str),   \
  (_str_arg_t[]) { _va_exp(_spa, __VA_ARGS__) },    \
  _va_count(__VA_ARGS__)                            \
)                                                   //

#define str_split_at(str, index)    slice_split_at(_s2r(str), index)
#define str_split_str(str, delim)   slice_split_str(_s2r(str), _s2r(delim))
#define str_split_char(str, delims) slice_split_char(_s2r(str), _s2r(delims))
#define str_split_any(str, delims)  slice_split_any(_s2r(str), delims)

#define str_split_whitespace(str)   slice_split_whitespace(_s2r(str))

// \brief Splits the string into an array of substrings based on the delimiter.
//
// \param str - The string to tokenize.
//
// \param delims - delimiters to tokenize the string with. Can be string types
//    (String, slice_t, char*), single chars ('x'), or string spans/Arrays.
//
// \returns an array of slices in the string, including the delimiters.
#define str_tokenize(str, ...)                      \
  istr_tokenize(_s2r(str),                          \
    (_str_arg_t[]) { _va_exp(_spa, __VA_ARGS__) },  \
    _va_count(__VA_ARGS__)                          \
  )                                                 //

#define str_tokenize_str(s, delim)  slice_tokenize_str(_s2r(s), _s2r(delim))
#define str_tokenize_char(s, dels)  slice_tokenize_char(_s2r(s), _s2r(dels))
#define str_tokenize_any(s, dels)   slice_tokenize_any(_s2r(s), dels)

// \brief Splits the string into two parts around the first matching delimiter.
//
// \param str - The string to partition.
//
// \param delims - delimiters to search for. Can be string types (String, char*,
//    slice_t), single chars ('x'), or slice containers (Array_slice,
//    span_slice_t).
//
// \returns a result type containing the left/right pair and found delimiter.
#define str_partition(str, ...)                     \
  istr_partition(_s2r(str),                         \
    (_str_arg_t[]) { _va_exp(_spa, __VA_ARGS__) },  \
    _va_count(__VA_ARGS__)                          \
  )                                                 //

#define str_partition_str(s, del)   slice_partition_str(_s2r(s), _s2r(del))
#define str_partition_char(s, dels) slice_partition_char(_s2r(s), _s2r(dels))
#define str_partition_any(s, any)   slice_partition_any(_s2r(s), _s2r(any))

// \brief Joins the provided strings, characters, and slices into one string.
//
// \param args - variadic set of strings, slices, chars, and slice spans.
//
// \returns a new string, which must be deleted later by the caller.
#define str_concat(...) istr_concat(                \
  (_str_arg_t[]) { _va_exp(_spa, __VA_ARGS__) },    \
  _va_count(__VA_ARGS__)                            \
)                                                   //

// \brief Joins an array of string slices into a new string, each separated by a
//    given delimiter.
//
// \param del - the delimiter to insert between each string in the array.
//   ex: (" + ", ["A", "B"]) will result in "A + B"
//
// \param args - variadic set of strings, slices, chars, and slice spans.
//
// \returns a new string, which must be deleted later by the caller.
#define str_join(del, ...) istr_join(_s2r(del),     \
  (_str_arg_t[]) { _va_exp(_spa, __VA_ARGS__) },    \
  _va_count(__VA_ARGS__)                            \
)                                                   //

#define str_replace(str, tok, w)    istr_replace(_s2r(str), _s2r(tok), _s2r(w))
#define str_replace_all(s, t, w)    istr_replace_all(_s2r(s), _s2r(t), _s2r(w))
#define str_prepend(str, length, c) istr_prepend(_s2r(str), length, c)
#define str_append(str, length, c)  istr_append(_s2r(str), length, c)

////////////////////////////////////////////////////////////////////////////////
// Output
////////////////////////////////////////////////////////////////////////////////

#define str_write(str)              slice_write(_s2r(str))

// \brief `String str_format(fmt, ...)`
// \brief Builds a new string from a format string and variable arguments.
//
// \brief The first parameter is always the format string, which takes argument
//    specifiers in the form {}, or with optional specifiers (denoted 
//    by brackets):
// 
//    {[index][![i|x|X|c]][:[+][<|^|>|=][#pad_char][width][.precision[+]]]}
//
//    - argument index:     {0}, {1}, ...
//    - type annotation:    {!x}, with index: {1!x}, with width {1!x:5}
//        - i, f: default
//        - x: hex
//        - X: HEX
//        - c: character
//        - b: binary
//        - o: octal (TODO)
//        - m: month, M: Month (TODO)
//        - d: day, D: Day (TODO)
//    - minimum width:      {:5}, with index: {1:5}
//    - alignment:          left {:<5}, right {:>5}, center {:^5}, ledger {:=5}
//        - < left align
//        - > right align
//        - ^ center align
//        - = ledger style (right-align, +/- signs left aligned)
//        - 0 ledger with leading zeroes {:05}, equivalent to {:#0=5}
//    - padding character:  {:#,5} fills whitespace with ',' instead of ' '
//    - sign for positives: {:+}, with index and width: {1:+5}
//    - precision (floats): {:.3}, use trailing zeroes on precision {:5.3+}
//    - exponent (TODO):    {:.3e}, {:.3E} (should be {!e}?)
//
// \returns a new string, which must be deleted later by the caller.
#define str_format(fmt, ...) istr_format(_s2r(fmt), \
  (_str_arg_t[]) { _va_exp(_sfa, __VA_ARGS__) },    \
  _va_count(__VA_ARGS__)                            \
)                                                   //

// \brief `void str_print(fmt, ...)`
// \brief Prints a formatted string as a log.
// \brief See str_format for details on formatting.
#define str_print(...) istr_print(                  \
  (_str_arg_t[]) { _va_exp(_sfa, __VA_ARGS__) },    \
  _va_count(__VA_ARGS__)                            \
)                                                   //

// \brief `void str_log(fmt, ...)`
// \brief Prints a formatted string as a log. Differes from str_print in that
//    values are not printed in release mode (TODO).
// \brief See str_format for details on formatting.
#define str_log(...) istr_log(                      \
  (_str_arg_t[]) { _va_exp(_sfa, __VA_ARGS__) },    \
  _va_count(__VA_ARGS__)                            \
)                                                   //

//String str_pad_left(slice_t str, index_t length, char c);
//String str_pad_right(slice_t str, index_t length, char c);

////////////////////////////////////////////////////////////////////////////////
// Internal string functions for slice input
////////////////////////////////////////////////////////////////////////////////

static inline index_t _str_size(slice_t s) { return s.size; }
static inline slice_t _str_slice_r(slice_t slice) { return slice; }
static inline slice_t _str_slice_st(const String str) {
  if (str) return str->slice;
  return str_empty->slice;
}

String      istr_copy(slice_t str);
String      istr_concat(_str_arg_t args[], index_t count);
String      istr_join(slice_t delim, _str_arg_t args[], index_t count);
String      istr_prepend(slice_t str, index_t length, char c);
String      istr_append(slice_t str, index_t length, char c);
String      istr_format(slice_t fmt, _str_arg_t args[], index_t count);
//String    istr_to_upper(slice_t str);
//String    istr_to_lower(slice_t str);

Array_slice istr_split(slice_t str, _str_arg_t args[], index_t count);
Array_slice istr_tokenize(slice_t st, _str_arg_t args[], index_t count);

//Array     istr_match(slice_t str, slice_t regex);
//Array     istr_parenthetize(slice_t str); // block out segments by parens? ([{}])
// for replace, start with basic string replace, maybe later look into adding regex support?
//    differentiate between regular strings and regex with the regular "a" vs "/a/"
//String    istr_replace(slice_t str, slice_t to_rep, slice_t with);
//String    istr_replace_all(slice_t str, slice_t r, slice_t w);
void        istr_print(slice_t fmt, _str_arg_t args[], index_t count);
void        istr_log(slice_t fmt, _str_arg_t args[], index_t count);

////////////////////////////////////////////////////////////////////////////////
// Variadic arg implementation for formatting and join
////////////////////////////////////////////////////////////////////////////////

static inline _str_arg_t _sarg_str(const String s) {
  return (_str_arg_t) { .type = _str_arg_slice, .slice = s->slice };
}

static inline _str_arg_t _sarg_slice(slice_t r) {
  return (_str_arg_t) { .type = _str_arg_slice, .slice = r };
}

static inline _str_arg_t _sarg_slice_ptr(const slice_t* r) {
  if (!r) r = &slice_empty;
  return (_str_arg_t) { .type = _str_arg_slice, .slice = *r };
}

static inline _str_arg_t _sarg_span(span_slice_t s) {
  return (_str_arg_t) { .type = _str_arg_span, .span = s };
}

static inline _str_arg_t _sarg_array(Array_slice a) {
  return (_str_arg_t) { .type = _str_arg_span, .span = a->span };
}

static inline _str_arg_t _sarg_c_str(const char* c_str) {
  if (!c_str) c_str = slice_empty.begin;
  return (_str_arg_t) {
    .type = _str_arg_slice, .slice = slice_from_c_str(c_str)
  };
}

static inline _str_arg_t _sarg_int(long long int i) {
  return (_str_arg_t) { .type = _str_arg_int, .i = i };
}

static inline _str_arg_t _sarg_unsigned(long long unsigned int i) {
  return (_str_arg_t) { .type = _str_arg_int, .i = (long long)i };
}

static inline _str_arg_t _sarg_float(double f) {
  return (_str_arg_t) { .type = _str_arg_float, .f = f };
}

// \brief str_format argument macro
#define _sfa(arg) _Generic((arg),       \
  slice_t:            _sarg_slice,      \
  String:             _sarg_str,        \
  slice_t*:           _sarg_slice_ptr,  \
  const slice_t*:     _sarg_slice_ptr,  \
  char*:              _sarg_c_str,      \
  const char*:        _sarg_c_str,      \
  int:                _sarg_int,        \
  long long:          _sarg_int,        \
  unsigned int:       _sarg_unsigned,   \
  unsigned long:      _sarg_unsigned,   \
  unsigned long long: _sarg_unsigned,   \
  double:             _sarg_float       \
)(arg)                                  //

// \brief str_split and str_join argument macro
#define _spa(arg) _Generic((arg),       \
  slice_t:            _sarg_slice,      \
  String:             _sarg_str,        \
  slice_t*:           _sarg_slice_ptr,  \
  const slice_t*:     _sarg_slice_ptr,  \
  span_slice_t:       _sarg_span,       \
  Array_slice:        _sarg_array,      \
  char*:              _sarg_c_str,      \
  const char*:        _sarg_c_str,      \
  int:                _sarg_int         \
)(arg)                                  //

#endif

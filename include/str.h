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

#include "slice.h"
#include "array_slice.h"

#include <stdarg.h>

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

// \brief Macro to coalesce a String, slice, or char* into a slice.
#define _s2r(str) _Generic((str), \
  slice_t:      slice_from_slice, \
  String:       slice_from_str,   \
  char*:        slice_from_c_str, \
  const char*:  slice_from_c_str  \
)(str)                            //

extern const String str_empty;
extern const String str_va_end;
extern const String str_true;
extern const String str_false;

static inline slice_t slice_from_str(String s) { return s->slice; }

#define str_write(str)              slice_write(_s2r(str))

#define str_copy(str)               istr_copy(_s2r(str))
String  str_build(const char* c_str, index_t length);
String  str_from_bool(bool b);
String  str_from_int(int i);
String  str_from_float(float f);

void    str_delete(String* str);

#define str_eq(lhs, rhs)            slice_eq(_s2r(lhs), _s2r(rhs))
#define str_starts_with(str, start) slice_starts_with(_s2r(str), _s2r(start))
#define str_ends_with(str, end)     slice_ends_with(_s2r(str), _s2r(end))
#define str_contains(str, check)    slice_contains(_s2r(str), _s2r(check))
bool    str_is_null_or_empty(const String str);

#define str_to_bool(str, out)       slice_to_bool(_s2r(str), out)
#define str_to_int(str, out)        slice_to_int(_s2r(str), out)
#define str_to_long(str, out)       slice_to_long(_s2r(str), out)
#define str_to_float(str, out)      slice_to_float(_s2r(str), out)
#define str_to_double(str, out)     slice_to_double(_s2r(str), out)

// \brief prefer s->size, but can be useful in cases where a function is needed.
//
// \returns s->size
#define str_size(str)               slice_size(_s2r(str))

// \brief Gets the start of the next instance of to_find in str, starting
//    at from_pos.
//
// \returns
//    The index in str of the match, or str.size if none is present.
#define str_index_of(str, to_find, from_pos) \
                    slice_index_of(_s2r(str), _s2r(to_find), from_pos)

// \brief Gets the start of the next instance of to_find in str, starting
//    at from_pos.
//
// \returns
//    The index in str of the match, or str.size if none is present.
#define str_index_of_char(str, to_find, from_pos) \
                    slice_index_of_char(_s2r(str), to_find, from_pos)

// \brief Gets a token as a substring of str described by the starting position
//    pos that ends with (not including) any delimeter character in to_find.
//
// \brief The value of pos is updated to the index after the found delimeter.
//    If no character was found in the string, the value is equal to str.size.
//
// \returns
//    A slice containing the token
#define str_token(str, to_find, pos) \
                    slice_token(_s2r(str), _s2r(to_find), pos)

// \brief Alias for str_index_of(str, to_find, 0)
#define str_find(str, to_find)      slice_find(_s2r(str), _s2r(to_find))

// \brief `slice str_substring(str, start, ?end)`
// \brief Gets a substring as a slice within the input string or slice.
// \brief Works like javascript string.slice.
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
#define str_substring(str, ...)     slice_substring(_s2r(str), __VA_ARGS__)
#define str_slice(str, ...)         slice_substring(_s2r(str), __VA_ARGS__)

#define str_trim(str)               slice_trim(_s2r(str))
#define str_trim_start(str)         slice_trim_start(_s2r(str))
#define str_trim_end(str)           slice_trim_end(_s2r(str))

// \brief Splits the string into an array of substrings based on the delimiter.
//
// \param str - The string to split into pieces.
//
// \param del - The substring to split the input along. Instances of the
//    delimiter are removed from the resulting substrings.
//
// \returns An array of slices whose lifetimes are bound to str.
//    The Array must be deleted by the user via arr_str_delete(&arr).
#define str_split(str, del)         slice_split(_s2r(str), _s2r(del))

// \brief Joins an array of string slices into a new string, each separated by a
//    given delimiter.
//
// \param del - the delimiter to insert between each string in the array.
//   ex: (" + ", ["A", "B"]) will result in "A + B"
//
// \param strings - The array of string slices to join.
//
// \returns a new string, which must be deleted later by the caller.
#define str_join(del, strings)      istr_join(_s2r(del), strings)
#define str_concat(left, right)     istr_concat(_s2r(left), _s2r(right))
#define str_replace(str, tok, w)    istr_replace(_s2r(str), _s2r(tok), _s2r(w))
#define str_replace_all(s, t, w)    istr_replace_all(_s2r(s), _s2r(t), _s2r(w))
#define str_prepend(str, length, c) istr_prepend(_s2r(str), length, c)
#define str_append(str, length, c)  istr_append(_s2r(str), length, c)

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
#define str_format(...)             _str_format(__VA_ARGS__, _str_fmtarg_end)

// \brief `void str_print(fmt, ...)`
// \brief Prints a formatted string as a log.
// \brief See str_format for details on formatting.
#define str_print(...)              _str_print(__VA_ARGS__, _str_fmtarg_end)

// \brief `void str_log(fmt, ...)`
// \brief Prints a formatted string as a log. Differes from str_print in that
//    values are not printed in release mode (TODO).
// \brief See str_format for details on formatting.
#define str_log(...)                _str_log(__VA_ARGS__, _str_fmtarg_end)

//String str_pad_left(slice_t str, index_t length, char c);
//String str_pad_right(slice_t str, index_t length, char c);

static inline index_t istr_size(slice_t s) { return s.size; }
static inline slice_t _str_slice_r(slice_t slice) { return slice; }
static inline slice_t _str_slice_st(const String str) {
  if (str) return str->slice;
  return str_empty->slice;
}

String      istr_copy(slice_t str);
//String    istr_to_upper(slice_t str);
//String    istr_to_lower(slice_t str);
//String    istr_to_title(slice_t str);
String      istr_concat(slice_t left, slice_t right);
String      istr_prepend(slice_t str, index_t length, char c);
String      istr_append(slice_t str, index_t length, char c);
String      istr_format(slice_t fmt, ...);

// Should be moved below to only be included if both headers are present.
// But also should be removed in favor of a variadic version that works more
//    like str_format and uses the same functions to process types including
//    Array_slice for improved flexibility.
String      istr_join(slice_t deliminter, const Array_slice strings);

//bool      istr_contains_any(slice_t str, slice_t check_chars);
//index_t   istr_index_of_last(slice_t str, slice_t find, index_t from);
//index_t   istr_find_last(slice_t str, slice_t to_find);
//Array     istr_match(slice_t str, slice_t regex);
//Array     istr_tokenize(slice_t str, const slice_t[] tokens);
//Array     istr_parenthetize(slice_t str); // block out segments by parens? ([{}])
// for replace, start with basic string replace, maybe later look into adding regex support?
//    differentiate between regular strings and regex with the regular "a" vs "/a/"
//String    istr_replace(slice_t str, slice_t to_rep, slice_t with);
//String    istr_replace_all(slice_t str, slice_t r, slice_t w);
void        istr_print(slice_t fmt, ...);
void        istr_log(slice_t fmt, ...);

#define _str_sub_args(str, start, end, ...) _s2r(str), (index_t)start, (index_t)end
#define _str_sub_a(str, ...) _str_sub_args(str, __VA_ARGS__, _s2r(str).size)
#define _str_substring(str, ...) istr_substring(_str_sub_a(str, __VA_ARGS__))

#define _str_format(fmt, ...) istr_format(_s2r(fmt), _va_exp(_sfa, __VA_ARGS__))
#define _str_print(fmt, ...) istr_print(_s2r(fmt), _va_exp(_sfa, __VA_ARGS__))
#define _str_log(fmt, ...) istr_log(_s2r(fmt), _va_exp(_sfa, __VA_ARGS__))

typedef enum _str_fmtType_t {
  _fmtArg_End,
  _fmtArg_Slice,
  _fmtArg_Int,
  _fmtArg_Float
} _str_fmtType_t;

typedef struct {
  _str_fmtType_t type;
  union {
    slice_t slice;
    ptrdiff_t i;
    double f;
  };
} _str_fmtArg_t;

extern const _str_fmtArg_t _str_fmtarg_end;

static inline _str_fmtArg_t _sarg_str(const String s) {
  return (_str_fmtArg_t) { .type = _fmtArg_Slice, .slice = s->slice };
}

static inline _str_fmtArg_t _sarg_slice(slice_t r) {
  return (_str_fmtArg_t) { .type = _fmtArg_Slice, .slice = r };
}

static inline _str_fmtArg_t _sarg_slice_ptr(const slice_t* r) {
  if (!r) r = &slice_empty;
  return (_str_fmtArg_t) { .type = _fmtArg_Slice, .slice = *r };
}

static inline _str_fmtArg_t _sarg_c_str(const char* c_str) {
  if (!c_str) c_str = slice_empty.begin;
  return (_str_fmtArg_t) {
    .type = _fmtArg_Slice, .slice = slice_from_c_str(c_str)
  };
}

static inline _str_fmtArg_t _sarg_int(long long int i) {
  return (_str_fmtArg_t) { .type = _fmtArg_Int, .i = i };
}

static inline _str_fmtArg_t _sarg_unsigned(long long unsigned int i) {
  return (_str_fmtArg_t) { .type = _fmtArg_Int, .i = (ptrdiff_t)i };
}

static inline _str_fmtArg_t _sarg_float(double f) {
  return (_str_fmtArg_t) { .type = _fmtArg_Float, .f = f };
}

static inline _str_fmtArg_t _sarg_arg(_str_fmtArg_t e) {
  PARAM_UNUSED(e); // should some kind of override be allowed?
  return _str_fmtarg_end;
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
  unsigned long long: _sarg_unsigned,   \
  double:             _sarg_float,      \
  _str_fmtArg_t:      _sarg_arg         \
)(arg)                                  //

#endif

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

#ifndef _MCLIB_STRING_H_
#define _MCLIB_STRING_H_

#include "types.h"

#include <stdarg.h>

#include "array.h"

#define _STR_RANGE_DEF(SRCA, SRCB)  \
  struct {                          \
    SRCA char*  SRCB begin;         \
    union {                         \
      index_s   SRCB length;        \
      index_s   SRCB size;          \
    };                              \
  }                                 //

#define _STR_RANGE_DEF_BODY(C_STR)  \
  .begin = C_STR,                   \
  .size = sizeof(C_STR)-1           //

// \brief StringRange is a basic immutable string segment containing the start
//    and size of a string.
//
// \brief The Range object does not have ownership of the string and related
//    functions will not allocate or free any memory for a returned Range.
//    Because of this, they're not suited for long-term tracking of a string
//    object unless that string is a compile-time constant string literal
//    (which you can wrap using the str_literal, R, or str_static macros).
//
// \brief A StringRange can be created referencing any of the following:
//    1) a constant string literal (using R, str_literal, or str_static).
//    2) a range within a String object (including the implicit str->range).
//    3) a or a range along a standard C-style char* string (str_range).
typedef _STR_RANGE_DEF(const,) StringRange;

// \brief Creates a string range from a string literal - ONLY use this for
//    literal string values (eg: "abc"). Will not copy or take ownership of the
//    string's memory.
// \brief Passing a char* to this will fail because the length is calculated at
//    compile time. For a runtime string range, use str_range(c_str).
//
// \param C_STRING_LITERAL - The string literal value.
//    Can accept either a string in double quotes, or a const static char[].
//
#define str_literal(C_STRING_LITERAL) ((StringRange) {  \
  _STR_RANGE_DEF_BODY(C_STRING_LITERAL)                 \
})                                                      //

// \brief Alias for str_literal(c_str).
// \brief Creates a string range from a string literal - ONLY use this for
//    literal string values (eg: "abc"). Will not copy or take ownership of the
//    string's memory.
// \brief Passing a char* to this will fail because the length is calculated at
//    compile time. For a runtime string range, use str_range(c_str).
//
// \param C_STRING_LITERAL - The string literal value.
//    Can accept either a string in double quotes, or a const static char[].
//
#define R(C_STR) str_literal(C_STR)

// \brief Similar to R macro, just doesn't include the typename because MSVC
//    can't handle that in some cases. Very annoying.
#define M(C_STRING_LITERAL) {           \
  _STR_RANGE_DEF_BODY(C_STRING_LITERAL) \
}                                       //

// \brief Used to allocate a static string from a string literal. This is only
//    necessary in MSVC because it can't understand initializer list casting.
//
// \param NAME - the name symbol for the static variable
//
// \param C_STR - the compile-time constant string literal value.
//
#define str_static(NAME, C_STR) static StringRange NAME = { \
  _STR_RANGE_DEF_BODY(C_STR)                                \
}                                                           //

// \brief String is a handle type pointing to an immutable string on the heap.
//
// \brief Unlike StringRange, a String object has ownership of the data in its
//    contained range. Any String object returned from a function will have to
//    later be freed via str_delete(). Any StringRange returned from a function
//    using the String's range will have its lifecycle bound to the String, and
//    will be invalid once the String object is deleted.
typedef struct _Str_Base {
  union {
    const StringRange range;
    _STR_RANGE_DEF(const, const);
  };
}* String;

#define con_type StringRange
#define con_prefix str
#include "array.h"
#undef con_type
#undef con_prefix
typedef Array_StringRange Array_StrR;

#ifdef _MSC_VER
// Annoyingly, MSVC for some reason detects the _Generic specifier as "unused".
#pragma warning ( disable : 4189 ) // local initialized but not referenced
#endif
// \brief Macro to coalesce a String, StringRange, or char* into a StringRange.
#define _s2r(S) _Generic((S),   \
  StringRange:  _str_range_r,   \
  String:       _str_range_st,  \
  char*:        str_range,      \
  const char*:  str_range       \
)(S)                            //

extern const String str_empty;
extern const String str_va_end;
extern const String str_true;
extern const String str_false;

StringRange str_range(const char* c_str);
StringRange str_range_s(const char* c_str, index_s length);

#define str_write(str)              istr_write(_s2r(str))

String  str_new(const char* c_str);
String  str_new_s(const char* c_str, index_s length);
#define str_copy(str)               istr_copy(_s2r(str))
String  str_from_bool(bool b);
String  str_from_int(int i);
String  str_from_float(float f);

void    str_delete(String* str);

#define str_eq(lhs, rhs)            istr_eq(_s2r(lhs), _s2r(rhs))
#define str_starts_with(str, start) istr_starts_with(_s2r(str), _s2r(start))
#define str_ends_with(str, end)     istr_ends_with(_s2r(str), _s2r(end))
#define str_contains(str, check)    istr_contains(_s2r(str), _s2r(check))

#define str_to_bool(str, out)       istr_to_bool(_s2r(str), out)
#define str_to_int(str, out)        istr_to_int(_s2r(str), out)
#define str_to_long(str, out)       istr_to_long(_s2r(str), out)
#define str_to_float(str, out)      istr_to_float(_s2r(str), out)
#define str_to_double(str, out)     istr_to_double(_s2r(str), out)

// \brief prefer s.size, but can be useful in cases where a function is needed.
//
// \returns s.size
#define str_size(str)               _str_size(_s2r(str))

// \brief Gets the start of the next instance of to_find in str, starting
//    at from_pos.
//
// \returns
//    The index in str of the match, or str.size if none is present.
#define str_index_of(str, to_find, from_pos) \
                    istr_index_of(_s2r(str), _s2r(to_find), from_pos)

// \brief Gets the start of the next instance of to_find in str, starting
//    at from_pos.
//
// \returns
//    The index in str of the match, or str.size if none is present.
#define str_index_of_char(str, to_find, from_pos) \
                    istr_index_of_char(_s2r(str), to_find, from_pos)

// \brief Gets a token as a substring of str described by the starting position
//    pos that ends with (not including) any delimeter character in to_find.
//
// \brief The value of pos is updated to the index after the found delimeter.
//    If no character was found in the string, the value is equal to str.size.
//
// \returns
//    A StringRange containing the token
#define str_token(str, to_find, pos) \
                    istr_token(_s2r(str), _s2r(to_find), pos)

// \brief Alias for str_index_of(str, to_find, 0)
#define str_find(str, to_find)      istr_find(_s2r(str), _s2r(to_find))

// \brief `StringRange str_substring(str, start, ?end)`
// \brief Gets a substring as a range within the input string range.
// \brief Works like javascript string.slice.
//
// \param str The string range to get a substring of. The returned substring's
//    lifetime will be dependent on the lifetime of str.
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
// \returns a StringRange as a substring of the input range.
#define str_substring(str, ...)     _str_substring(str, __VA_ARGS__)
#define str_slice(str, ...)         _str_substring(str, __VA_ARGS__)

#define str_trim(str)               istr_trim(_s2r(str))
#define str_trim_start(str)         istr_trim_start(_s2r(str))
#define str_trim_end(str)           istr_trim_end(_s2r(str))

// \brief Splits the string into an array of substrings based on the delimiter.
//
// \param str - The string to split into pieces.
//
// \param del - The substring to split the input along. Instances of the
//    delimiter are removed from the resulting substrings.
//
// \returns An array of StringRanges whose lifetimes are bound to str.
//    The Array must be deleted by the user via arr_str_delete(&arr).
#define str_split(str, del)         istr_split(_s2r(str), _s2r(del))

// \brief Joins an array of string ranges into a new string, each separated by a
//    given delimiter.
//
// \param del - the delimiter to insert between each string in the array.
//   ex: (" + ", ["A", "B"]) will result in "A + B"
//
// \param strings - The array of string ranges to join.
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

//String str_pad_left(StringRange str, index_s length, char c);
//String str_pad_right(StringRange str, index_s length, char c);

static inline index_s     _str_size(StringRange s) { return s.size; }
static inline StringRange _str_range_r(StringRange range) { return range; }
static inline StringRange _str_range_st(const String str) {
  if (str) return str->range;
  return str_empty->range;
}

void        istr_write(StringRange str);
String      istr_copy(StringRange str);
bool        istr_eq(StringRange lhs, StringRange rhs);
bool        istr_starts_with(StringRange str, StringRange starts);
bool        istr_ends_with(StringRange str, StringRange ends);
bool        istr_contains(StringRange str, StringRange check);
bool        istr_contains_char(StringRange str, char check);
bool        istr_contains_any(StringRange str, StringRange check_chars);
bool        istr_to_bool(StringRange str, bool* out_bool);
bool        istr_to_int(StringRange str, int* out_int);
bool        istr_to_long(StringRange str, index_s* out_int);
bool        istr_to_float(StringRange str, float* out_float);
bool        istr_to_double(StringRange str, double* out_float);
//String    istr_to_upper(StringRange str);
//String    istr_to_lower(StringRange str);
//String    istr_to_title(StringRange str);
index_s     istr_index_of_char(StringRange str, char c, index_s from);
index_s     istr_index_of(StringRange str, StringRange to_find, index_s from);
StringRange istr_token(StringRange str, StringRange del_chrs, index_s* pos);
//index_s   istr_index_of_last(StringRange str, StringRange find, index_s from);
index_s     istr_find(StringRange str, StringRange to_find);
//index_s   istr_find_last(StringRange str, StringRange to_find);
//Array     istr_match(StringRange str, StringRange regex);
StringRange istr_substring(StringRange str, index_s start, index_s end);
StringRange istr_trim(StringRange str);
StringRange istr_trim_start(StringRange str);
StringRange istr_trim_end(StringRange str);
Array_StrR  istr_split(StringRange str, StringRange del);
//Array     istr_tokenize(StringRange str, const StringRange[] tokens);
//Array     istr_parenthetize(StringRange str); // block out segments by parens? ([{}])
String      istr_join(StringRange deliminter, const Array_StringRange strings);
String      istr_concat(StringRange left, StringRange right);
// for replace, start with basic string replace, maybe later look into adding regex support?
//    differentiate between regular strings and regex with the regular "a" vs "/a/"
//String    istr_replace(StringRange str, StringRange to_rep, StringRange with);
//String    istr_replace_all(StringRange str, StringRange r, StringRange w);
String      istr_prepend(StringRange str, index_s length, char c);
String      istr_append(StringRange str, index_s length, char c);
String      istr_format(StringRange fmt, ...);
void        istr_print(StringRange fmt, ...);
void        istr_log(StringRange fmt, ...);

#define _str_sub_args(str, start, end, ...) _s2r(str), (index_s)start, (index_s)end
#define _str_sub_a(str, ...) _str_sub_args(str, __VA_ARGS__, _s2r(str).size)
#define _str_substring(str, ...) istr_substring(_str_sub_a(str, __VA_ARGS__))

#define _str_format(fmt, ...) istr_format(_s2r(fmt), _va_exp(_sfa, __VA_ARGS__))
#define _str_print(fmt, ...) istr_print(_s2r(fmt), _va_exp(_sfa, __VA_ARGS__))
#define _str_log(fmt, ...) istr_log(_s2r(fmt), _va_exp(_sfa, __VA_ARGS__))

enum _Str_FmtArg_Type {
  _Str_FmtArg_End,
  _Str_FmtArg_StringRange,
  _Str_FmtArg_Int,
  _Str_FmtArg_Float,
};

typedef struct {
  int type;
  union {
    StringRange range;
    ptrdiff_t i;
    double f;
  };
} _Str_FmtArg;

extern const _Str_FmtArg _str_fmtarg_end;

static inline _Str_FmtArg _sarg_str(const String s) {
  return (_Str_FmtArg) { .type = _Str_FmtArg_StringRange, .range = s->range };
}

static inline _Str_FmtArg _sarg_range(StringRange r) {
  return (_Str_FmtArg) { .type = _Str_FmtArg_StringRange, .range = r };
}

static inline _Str_FmtArg _sarg_c_str(const char* c_str) {
  return (_Str_FmtArg) {
    .type = _Str_FmtArg_StringRange,
    .range = str_range(c_str)
  };
}

static inline _Str_FmtArg _sarg_int(long long int i) {
  return (_Str_FmtArg) { .type = _Str_FmtArg_Int, .i = i };
}

static inline _Str_FmtArg _sarg_unsigned(long long unsigned int i) {
  return (_Str_FmtArg) { .type = _Str_FmtArg_Int, .i = (ptrdiff_t)i };
}

static inline _Str_FmtArg _sarg_float(double f) {
  return (_Str_FmtArg) { .type = _Str_FmtArg_Float, .f = f };
}

static inline _Str_FmtArg _sarg_arg(_Str_FmtArg e) {
  return e;
}

// \brief str_format argument macro
#define _sfa(arg) _Generic((arg),       \
  StringRange:        _sarg_range,      \
  String:             _sarg_str,        \
  char*:              _sarg_c_str,      \
  const char*:        _sarg_c_str,      \
  int:                _sarg_int,        \
  long long:          _sarg_int,        \
  unsigned int:       _sarg_unsigned,   \
  unsigned long long: _sarg_unsigned,   \
  double:             _sarg_float,      \
  _Str_FmtArg:        _sarg_arg         \
)(arg)                                  //

#endif

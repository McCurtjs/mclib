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

#ifndef _MCLIB_SLICE_H_
#define _MCLIB_SLICE_H_

#include "types.h"

// \brief slice_t is a basic immutable string segment containing the start
//    and size of a string.
//
// \brief The slice_t object does not have ownership of the string and related
//    functions will not allocate or free any memory for a returned Range.
//    Because of this, they're not suited for long-term tracking of a string
//    object unless that string is a compile-time constant string literal
//    (which you can wrap using the str_literal, R, or str_static macros).
//
// \brief A slice can be created referencing any of the following:
//    1) a constant string literal (using S, str_literal, or str_static).
//    2) a slice within a String object (including the implicit str->range).
//    3) a or a slice along a standard C-style char* string (slice()).
typedef struct slice_t {
  const char* begin;
  union {
    index_t   length;
    index_t   size;
  };
} slice_t;

// String Constants

// \brief constant slice for an empty string
extern const slice_t slice_empty;

// \brief constant slice for a "true" string
extern const slice_t slice_true;

// \brief constant slice for a "false" string
extern const slice_t slice_false;

// \brief Similar to slice macro, just doesn't include the typename because MSVC
//    can't handle that in some cases. Very annoying. (prefer "slice" or "S").
#define slice_s(C_STR_LITERAL) {                                              \
  .begin = C_STR_LITERAL,                                                     \
  .size = sizeof(C_STR_LITERAL) - 1                                           \
}                                                                             //

// \brief Creates a string slice from a string literal - ONLY use this for
//    literal string values (eg: slice("abc")). Will not copy or take ownership of the
//    string's memory.
// \brief Passing a char* to this will fail because the length is calculated at
//    compile time. For a runtime slice of a null-terminated string,
//    use slice_from_ptr().
//
// \param C_STRING_LITERAL - The string literal value.
//    Can accept either a string in double quotes, or a const static char[].
//
#define slice(C_STR_LITERAL) ((slice_t) slice_s(C_STR_LITERAL))

// \brief Alias for slice(c_str)
#define S(C_STR_LITERAL) slice(C_STR_LITERAL)

// \brief Alias for slice_body for MSVC
#define M(C_STR_LITERAL) slice(C_STR_LITERAL)

// \brief Used to allocate a static string from a string literal. This is only
//    necessary in MSVC because it can't understand initializer list casting.
//
// \param NAME - the name symbol for the static variable
//
// \param C_STR_LITERAL - the compile-time constant string literal value.
//
#define slice_static(NAME, C_STR_LITERAL) static slice_t NAME = {             \
  slice_body(C_STR_LITERAL)                                                   \
}                                                                             //

// \brief Builds a slice from a c_string using strlen to find the length.
slice_t slice_build(const char* c_str);

// \brief Builds a slice from a c_string using the given length.
// 
// \brief Preferable to just use { .begin = str, .length = len }
static inline slice_t slice_build_s(const char* c_str, index_t length) {
  assert(length >= 0);
  assert(c_str == NULL ? length == 0 : true);
  return (slice_t) { .begin = c_str, .length = length };
}

// \brief Just a passthrough - takes a slice, returns the same slice.
// \brief Used in _Generic statements.
static inline slice_t slice_from_slice(slice_t s) { return s; }

static inline index_t slice_size(slice_t s) { return s.size; }

bool        slice_eq(slice_t lhs, slice_t rhs);
bool        slice_starts_with(slice_t str, slice_t starts);
bool        slice_ends_with(slice_t str, slice_t ends);
bool        slice_contains(slice_t str, slice_t check);
bool        slice_contains_char(slice_t str, char check);
bool        slice_is_empty(slice_t str);
//bool      slice_contains_any(slice_t str, slice_t check_chars);
bool        slice_to_bool(slice_t str, bool* out_bool);
bool        slice_to_int(slice_t str, int* out_int);
bool        slice_to_long(slice_t str, index_t* out_int);
bool        slice_to_float(slice_t str, float* out_float);
bool        slice_to_double(slice_t str, double* out_float);
bool        slice_find(slice_t str, slice_t to_find, slice_t* out_slice);
index_t     slice_index_of(slice_t str, slice_t to_find, index_t from);
index_t     slice_index_of_char(slice_t str, char c, index_t from);
slice_t     slice_token(slice_t str, slice_t del_chrs, index_t* pos);
//span_byte_t slice_to_span(slice_t str);
//index_t   slice_index_of_last(slice_t str, slice_t find, index_t from);
//index_t   slice_find_last(slice_t str, slice_t to_find);
//Array     slice_match(slice_t str, slice_t regex);
slice_t    islice_substring(slice_t str, index_t start, index_t end);
slice_t     slice_trim(slice_t str);
slice_t     slice_trim_start(slice_t str);
slice_t     slice_trim_end(slice_t str);
//Array_slice slice_split(slice_t str, slice_t del);
//Array     slice_tokenize(slice_t str, const slice_t[] tokens);
//Array     slice_parenthetize(slice_t str); // block out segments by parens? ([{}])
void        slice_write(slice_t str);

// slice_substring argument default
#define _slice_sub_args(str, start, end, ...) (str), (start), (end)
#define _slice_sub_v(str, ...) MCOMP(_slice_sub_args, (str, __VA_ARGS__, str.size))
#define slice_substring(str, ...) islice_substring(_slice_sub_v(str, __VA_ARGS__))

#include "span_byte.h"

//span_byte_t slice_to_span(slice_t slice);
slice_t span_byte_to_slice(span_byte_t span);

#include "array_slice.h"

Array_slice slice_split(slice_t str, slice_t del);

#endif

/*

Ideas for Arena allocator:

arena_t or Arena stores block of memory (either allocated on the heap, or declared on the stack).

struct arena_t {
  byte* base; // null for malloc
  index_t size;
  struct arena_t* prev;
  bool is_auto;
};

Arenas operate as a stack, making a new one takes over allocations until released. If "base" is
    null, use default malloc. Previous allocator contains current, unless "default" is re-added
    onto the stack.

If an arena runs out of space, instead of failing, a new arena could be added to the stack with 
    the "is_auto" flag set. When popping the stack to remove an allocator, if the flag is set
    keep popping until the top of the stack has is_auto == false (so it functions as if it was
    just one allocator).
Add uint max_autos that sets how many auto allocators can be created? (decrement by one each time
    another is added to the stack).

*/

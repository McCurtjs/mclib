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

#ifndef MCLIB_SLICE_H_
#define MCLIB_SLICE_H_

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

// Create pair_slice_t
#define tuple_type slice_t
#include "pair.h"
#undef tuple_type

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

// \brief slice constant for an empty string
extern const slice_t slice_empty;

// \brief slice constant for a "true" string
extern const slice_t slice_true;

// \brief slice constant for a "false" string
extern const slice_t slice_false;

// \brief slice constant for a string representing whitespace characters
extern const slice_t slice_whitespace;

// \brief slice constant for a " " string
extern const slice_t slice_space;

// \brief slice constant for a newline character
extern const slice_t slice_newline;

// \brief slice constant for a tab character
extern const slice_t slice_tab;

////////////////////////////////////////////////////////////////////////////////
// User-provided hook for output
////////////////////////////////////////////////////////////////////////////////

// \brief The base print function for McLib. This can be pointed to a custom
//    externally provided printer instead depending on context.
//
// \brief A default printer is provided using printf unless the build is set to
//    WASM or if MCLIB_NO_STDIO is defined.
//
// \param str - the slice to print. May not be null-terminated, but should only
//    print str.length characters in the string.
extern void (*slice_write)(slice_t str);

////////////////////////////////////////////////////////////////////////////////
// Construction
////////////////////////////////////////////////////////////////////////////////

// \brief Creates a string slice from a string literal (eg: S("Abc")).
//
// \param C_STR_LITERAL - The string literal value.
//    Can accept either a string in double quotes, or a const static char[].
#define S(C_STR_LITERAL) ((slice_t) {                                         \
  .begin = C_STR_LITERAL,                                                     \
  .size = sizeof(C_STR_LITERAL) - 1                                           \
})                                                                            //

// \brief Same as S, but omits the type specifier so it can be used as a
//    constant expression.
//
// \param C_STR_LITERAL - The string literal value - either a string in double
//    quotes, or a char[].
#define STRL(C_STR_LITERAL) {                                                 \
  .begin = (C_STR_LITERAL),                                                   \
  .size = sizeof(C_STR_LITERAL) - 1                                           \
}                                                                             //

// \brief Used to allocate a static string from a string literal. This is only
//    necessary in MSVC because it can't understand initializer list casting.
//
// \param NAME - the name symbol for the static variable
//
// \param C_STR_LITERAL - the compile-time constant string literal value.
#define slice_static(NAME, C_STR_LITERAL) static slice_t NAME = {             \
  slice_body(C_STR_LITERAL)                                                   \
}                                                                             //

// \brief Builds a slice from a c_string using strlen to find the length.
//
// \brief In general, prefer the S("str") macro to avoid use of strlen.
slice_t slice_from_c_str(const char* c_str);

// \brief Builds a slice from a c_string using the given length.
static inline slice_t slice_build(const char* c_str, index_t length) {
  assert(length >= 0);
  assert(c_str == NULL ? length == 0 : true);
  return (slice_t) { .begin = c_str, .length = length };
}

// \brief Just a passthrough - takes a slice, returns the same slice.
// \brief Used in _Generic statements.
static inline slice_t slice_from_slice(slice_t s) { return s; }

static inline index_t slice_size(slice_t s) { return s.size; }

////////////////////////////////////////////////////////////////////////////////
// Return result tuples
////////////////////////////////////////////////////////////////////////////////

typedef struct res_token_t {
  union {
    pair_slice_t pair;
    struct {
      slice_t token;
      slice_t delimiter;
    };
  };
} res_token_t;

// Create partition_slice_t
#define tuple_type slice_t
#define tuple_pair_type pair_slice_t
#include "partition.h"
#undef tuple_pair_type
#undef tuple_type

////////////////////////////////////////////////////////////////////////////////
// Usage
////////////////////////////////////////////////////////////////////////////////

bool              slice_to_bool(slice_t str, bool* out_bool);
bool              slice_to_int(slice_t str, int* out_int);
bool              slice_to_long(slice_t str, index_t* out_int);
bool              slice_to_float(slice_t str, float* out_float);
bool              slice_to_double(slice_t str, double* out_float);

int               slice_compare(slice_t lhs, slice_t rhs);
//int             slice_compare_ci(slice_t lhs, slice_t rhs);
bool              slice_eq(slice_t lhs, slice_t rhs);
//bool            slice_eq_ci(slice_t lhs, slice_t rhs);
bool              slice_starts_with(slice_t str, slice_t beginning);
bool              slice_ends_with(slice_t str, slice_t ending);
bool              slice_contains(slice_t str, slice_t target);
bool              slice_contains_char(slice_t str, slice_t targets);
bool              slice_is_empty(slice_t str);
//bool            slice_is_alpha(slice_t str);
//bool            slice_is_decimal(slice_t str);
//bool            slice_is_integer(slice_t str);
//index_t         slice_count_str(slice_t str, slice_t target);
//index_t         slice_count_char(slice_t str, slice_t targets);

bool              slice_find_str(slice_t str, slice_t target, slice_t* o_found);
bool              slice_find_char(slice_t str, slice_t tgt, slice_t* o_found);
bool              slice_find_last_str(slice_t s, slice_t tgt, slice_t* o_found);
bool              slice_find_last_char(slice_t s, slice_t t, slice_t* o_found);
index_t           slice_index_of_str(slice_t str, slice_t target);
index_t           slice_index_of_char(slice_t str, slice_t targets);
index_t           slice_index_of_last_str(slice_t str, slice_t target);
index_t           slice_index_of_last_char(slice_t str, slice_t targets);

res_token_t       slice_token_str(slice_t str, slice_t delim, index_t* pos);
res_token_t       slice_token_char(slice_t str, slice_t delims, index_t* pos);

pair_slice_t      slice_split_at(slice_t str, index_t index);
partition_slice_t slice_partition_str(slice_t str, slice_t delim);
partition_slice_t slice_partition_char(slice_t str, slice_t delims);

// get next numeric value? number string would go in the delimiter value
//res_partition slice_partition_number(slice_t str);

slice_t           slice_substring(slice_t str, index_t start, index_t end);
slice_t           slice_drop(slice_t str, index_t count);
slice_t           slice_take(slice_t str, index_t count);
slice_t           slice_trim(slice_t str);
slice_t           slice_trim_start(slice_t str);
slice_t           slice_trim_end(slice_t str);
//slice_t         slice_trim_char(slice_t str, slice_t chars);
//slice_t         slice_trim_char_start(slice_t str, slice_t chars);
//slice_t         slice_trim_char_end(slice_t str, slice_t chars);

hash_t            slice_hash(slice_t str);

int               slice_compare_vptr(const void* lh, const void* rh, size_t na);
hash_t            slice_hash_vptr(const void* slice, index_t unused);
void*             slice_copy_vptr(void* dst, const void* src, size_t unused);
void              slice_delete_vptr(void* str);

////////////////////////////////////////////////////////////////////////////////
// Additional functions brought in when other headers are present
////////////////////////////////////////////////////////////////////////////////

// If byte spans are available, re-include to get slice functions
#ifdef MCLIB_SPAN_BYTE_
# include "span_byte.h"
#endif

#ifdef MCLIB_ARRAY_BYTE_
# include "array_byte.h"
#endif

#endif

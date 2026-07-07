/*******************************************************************************
* MIT License
*
* Copyright (c) 2026 Curtis McCoy
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

#ifndef MCLIB_ARRAY_BYTE_
#define MCLIB_ARRAY_BYTE_

#include "view_byte.h"
#include "span_byte.h"

#define con_type byte
#define con_span_type span_byte_t
#define con_view_type view_byte_t
#include "array.h"
#undef con_span_type
#undef con_view_type
#undef con_type

span_byte_t arr_byte_append_int(Array_byte, long long int i);
span_byte_t arr_byte_append_float(Array_byte, double f, int precision);
void        arr_byte_align(Array_byte, index_t offset);

#endif

////////////////////////////////////////////////////////////////////////////////
// Additional functions for byte arrays when slices are included
////////////////////////////////////////////////////////////////////////////////

#ifdef MCLIB_SLICE_H_
# ifndef MCLIB_ARR_BYTE_SLICE_FNS_
# define MCLIB_ARR_BYTE_SLICE_FNS_

static inline slice_t slice_from_arr(Array_byte arr) {
  assert(arr);
  return slice_build((char*)arr->begin, arr->size);
}

span_byte_t iarr_byte_append(Array_byte, slice_t slice);
span_byte_t iarr_byte_append_repeat(Array_byte, slice_t slice, index_t count);

static inline Array_byte arr_byte_copy_slice(slice_t slice) {
  Array_byte ret = arr_byte_new_reserve(slice.size);
  iarr_byte_append(ret, slice);
  return ret;
}

#ifndef _s2r
# define _s2r(X) X
#endif

# define arr_byte_copy_str(str)      arr_byte_copy_slice(_s2r(str))

# define arr_byte_append(arr, slice) iarr_byte_append(arr, _s2r(slice))

# define arr_byte_appened_repeat(arr, slice, count)                           \
           iarr_byte_append_repeat(arr, _s2r(slice), count)                   //

# endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Additional functions for byte arrays when Strings are included
////////////////////////////////////////////////////////////////////////////////

#ifdef MCLIB_STRING_H_
# ifndef MCLIB_ARR_BYTE_STRING_FNS_
# define MCLIB_ARR_BYTE_STRING_FNS_

Array_byte    arr_byte_new_reserve_str(index_t capacity);
array_byte_t  arr_byte_build_reserve_str(index_t capacity);
String        arr_byte_finish_str(Array_byte);
String        arr_byte_release_str(Array_byte*);

static inline Array_byte arr_byte_new_str(void) {
  return arr_byte_new_reserve_str(0);
}

static inline array_byte_t arr_byte_build_str(void) {
  return arr_byte_build_reserve_str(0);
}

void iarr_byte_append_format(
  Array_byte output, slice_t fmt, _str_arg_t args[], index_t argc
);

# define arr_byte_append_format(out, fmt, ...) iarr_byte_append_format(       \
  (out),                                                                      \
  _s2r(fmt),                                                                  \
  (_str_arg_t[]) { _va_exp(_sfa, __VA_ARGS__) },                              \
  _va_count(__VA_ARGS__)                                                      \
)                                                                             //

# endif
#endif

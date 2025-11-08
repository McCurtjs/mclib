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

#ifndef MCLIB_ARRAY_BYTE_
#define MCLIB_ARRAY_BYTE_

#include "span_byte.h"

#define con_type byte
#define con_span_type span_byte_t
#define con_view_type view_byte_t
#include "array.h"
#undef con_span_type
#undef con_view_type
#undef con_type

span_byte_t arr_byte_append_int(Array_byte arr, long long int i);
span_byte_t arr_byte_append_float(Array_byte arr, double f, int precision);

#endif

#ifdef MCLIB_SLICE_H_
# ifndef MCLIB_ARR_BYTE_SLICE_FNS_
#   define MCLIB_ARR_BYTE_SLICE_FNS_

span_byte_t iarr_byte_append(Array_byte arr, slice_t slice);

#   define arr_byte_append(arr, slice) iarr_byte_append(arr, slice)

# endif
#endif


#ifdef MCLIB_STRING_H_
# ifndef MCLIB_ARR_BYTE_STRING_FNS_
#   define MCLIB_ARR_BYTE_STRING_FNS_

#   undef arr_byte_append
#   define arr_byte_append(arr, slice) iarr_byte_append(arr, _s2r(slice))

void arr_byte_append_format(
  Array_byte output, slice_t fmt, _str_arg_t args[], index_t argc
);

# endif
#endif

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

#endif

#ifdef MCLIB_SLICE_H_
# ifndef MCLIB_ARR_BYTE_SLICE_FNS_
# define MCLIB_ARR_BYTE_SLICE_FNS_

static inline void arr_byte_append(Array_byte arr, slice_t slice) {
  span_byte_t bytes = arr_byte_emplace_back_range(arr, slice.size);
  span_byte_copy_range(bytes, slice_to_view(slice), slice.size);
}

# endif
#endif

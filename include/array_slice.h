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

#ifndef MCLIB_ARRAY_SLICE_
#define MCLIB_ARRAY_SLICE_

#include "span_slice.h"

#define con_type slice_t
#define con_prefix slice
#include "array.h"
#undef con_type
#undef con_prefix

// Specialty Array_slice functions

Array_slice slice_split_str(slice_t str, slice_t delim);
Array_slice slice_split_char(slice_t str, slice_t delims);
Array_slice slice_split_any(slice_t str, span_slice_t delims);
Array_slice slice_tokenize_str(slice_t str, slice_t delim);
Array_slice slice_tokenize_char(slice_t str, slice_t delims);
Array_slice slice_tokenize_any(slice_t str, span_slice_t delims);

//Array_slice slice_match(slice_t str, slice_t regex);

#endif

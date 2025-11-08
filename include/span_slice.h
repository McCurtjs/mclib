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

#ifndef MCLIB_SPAN_SLICE_
#define MCLIB_SPAN_SLICE_

#include "slice.h"

#define con_type slice_t
#define con_prefix slice
#define con_view_type view_slice_t
#include "view.h"
#include "span.h"
#undef con_type
#undef con_prefix
#undef con_view_type

bool            slice_contains_any(slice_t str, span_slice_t any);
bool            slice_find_any(slice_t str, span_slice_t any, slice_t* out_s);
bool            slice_find_last_any(slice_t s, span_slice_t a, slice_t* out_s);
index_t         slice_index_of_any(slice_t str, span_slice_t any);
index_t         slice_index_of_last_any(slice_t str, span_slice_t any);
res_token_t     slice_token_any(slice_t str, span_slice_t any, index_t* pos);
partition_slice_t slice_partition_any(slice_t str, span_slice_t delims);

#endif

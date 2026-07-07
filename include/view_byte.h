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

#ifndef MCLIB_VIEW_BYTE_
#define MCLIB_VIEW_BYTE_
#define con_type byte
#include "view.h"
#undef con_type
#endif

// Specialty span_byte_t functions

#ifdef MCLIB_SLICE_H_
# ifndef MCLIB_VIEW_BYTE_SLICE_FNS_
# define MCLIB_VIEW_BYTE_SLICE_FNS_

slice_t view_to_slice(view_t view);
view_byte_t slice_to_view(slice_t slice);

static inline slice_t view_byte_to_slice(view_byte_t view) {
  return view_to_slice(view.base);
}

# endif
#endif

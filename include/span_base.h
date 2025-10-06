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

#ifndef MCLIB_SPAN_BASE_H_
#define MCLIB_SPAN_BASE_H_

#include "types.h"

// base span_t (void) struct is defined separately from the span.h file because
// it is also independently used by the Array class.

typedef struct span_t {
  void* begin;
  void* end;
} span_t;

#define SPAN(S) { .begin = (S), .end = (S) + ARRAY_COUNT(S) }

#define span_foreach(VAR, SPAN)                                               \
  VAR = SPAN.begin;                                                           \
  for (; (byte*)VAR < (byte*)SPAN.end; ++VAR)                                 //

#define span_foreach_index(VAR, INDEX, SPAN)                                  \
  VAR = SPAN.begin;                                                           \
  for (index_t INDEX = 0; (byte*)VAR < (byte*)SPAN.end; ++VAR, ++INDEX)       //

#endif

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

#include "span.h"

#include <stdlib.h>
#include <memory.h>

const void* empty = NULL;

const span_t span_empty = { .begin = &empty, .end = &empty };

void ispan_set_bytes(span_t span, byte b) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  size_t size = end - begin;
  memset(begin, b, size);
}

bool ispan_eq(span_t a, span_t b) {
  index_t size = ispan_size_bytes(a);
  if (size != ispan_size_bytes(b)) {
    return false;
  }
  return memcmp(a.begin, b.begin, size) == 0;
}

bool ispan_eq_deep(span_t lhs, span_t rhs, index_t el_size, compare_fn cmp) {
  index_t size = ispan_size(lhs, el_size);
  if (size != ispan_size(rhs, el_size)) {
    return false;
  }
  byte* p_lhs = lhs.begin;
  byte* p_rhs = rhs.begin;
  for (index_t i = 0; i < size; ++i) {
    if (cmp(p_lhs, p_rhs) != 0) {
      return false;
    }
    p_lhs += el_size;
    p_rhs += el_size;
  }
  return true;
}

void ispan_sort(span_t span, index_t element_size, compare_fn cmp) {
  index_t length = ispan_size(span, element_size);
  qsort(span.begin, length, element_size, cmp);
}

#include "slice.h"
#include "span_byte.h"

slice_t span_byte_to_slice(span_byte_t span) {
  assert(span.end >= span.begin);
  slice_t ret = { .begin = (char*)span.begin, .size = (span.end - span.begin) };
  if (span.end > span.begin && *(span.end - 1) == '\0') {
    ret.size -= 1;
  }
  return ret;
}

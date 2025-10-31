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

#include "span.h"

#include <stdlib.h>
#include <memory.h>

const span_t span_empty = { .begin = NULL, .end = NULL };
const view_t view_empty = { .begin = NULL, .end = NULL };

const void* view_ref(view_t view, index_t index, index_t element_size) {
  if (view_is_empty(view)) return NULL;
  index_t size = view_size(view, element_size);
  if (index < 0) index = size + index;
  if (index < 0 || index >= size) return NULL;
  return (const byte*)view.begin + index * element_size;
}

void* span_ref(span_t span, index_t index, index_t element_size) {
  if (span_is_empty(span)) return NULL;
  index_t size = span_size(span, element_size);
  if (index < 0) index = size + index;
  if (index < 0 || index >= size) return NULL;
  return (byte*)span.begin + index * element_size;
}

////////////////////////////////////////////////////////////////////////////////

view_t view_subview(view_t view, index_t start, index_t end, index_t el_size) {
  if (view_is_empty(view)) return view;
  index_t size = view_size(view, el_size);
  if (start < 0) start = size + start;
  if (start < 0) start = 0;
  if (end > size) end = size;
  if (end < 0) end = size + end;
  if (end < start) end = start;
  view.end = (const byte*)view.begin + end * el_size;
  view.begin = (const byte*)view.begin + start * el_size;
  return view;
}

span_t span_subspan(span_t span, index_t start, index_t end, index_t el_size) {
  view_t ret = view_subview(span.view, start, end, el_size);
  return *((span_t*)&ret);
}

view_t view_drop(view_t view, index_t count, index_t element_size) {
  if (view_is_empty(view)) return view;
  index_t size = view_size(view, element_size);
  const byte* begin = view.begin;
  const byte* end = view.end;
  if (count >= 0) {
    if (count >= size) begin = end;
    else begin += count * element_size;
  } else {
    if (-count >= size) end = begin;
    else end += count * element_size;
  }
  return (view_t) { .begin = begin, .end = end };
}

span_t span_drop(span_t span, index_t count, index_t element_size) {
  view_t ret = view_drop(span.view, count, element_size);
  return *((span_t*)&ret);
}

view_t view_take(view_t view, index_t count, index_t element_size) {
  if (view_is_empty(view)) return view;
  index_t size = view_size(view, element_size);
  const byte* begin = view.begin;
  const byte* end = view.end;
  if (count >= 0) {
    if (count >= size) return view;
    end = begin + count * element_size;
  } else {
    if (-count >= size) return view;
    begin = end + count * element_size;
  }
  return (view_t) { .begin = begin, .end = end };
}

span_t span_take(span_t span, index_t count, index_t element_size) {
  view_t ret = view_take(span.view, count, element_size);
  return *((span_t*)&ret);
}

pair_view_t view_split(view_t view, index_t pivot, index_t element_size) {
  if (view_is_empty(view)) return (pair_view_t) { view, view };
  index_t size = view_size(view, element_size);
  if (pivot >= size) return (pair_view_t) { view, view_empty };
  if (pivot < 0) pivot += size;
  if (pivot <= 0) return (pair_view_t) { view_empty, view };
  const byte* middle = (const byte*)view.begin + pivot * element_size;
  return (pair_view_t) {
    .left = (view_t){ view.begin, middle },
      .right = (view_t){ middle, view.end }
  };
}

pair_span_t span_split(span_t span, index_t pivot, index_t element_size) {
  pair_view_t ret = view_split(span.view, pivot, element_size);
  return *((pair_span_t*)&ret);
}

partition_view_t view_partition(
  view_t view, const void* del, compare_nosize_fn compare, index_t element_size
) {
  if (view_is_empty(view)) return (partition_view_t) { view, view, NULL };
  const byte* item = view.begin;
  while (item < (const byte*)view.end) {
    if (!compare(item, del)) {
      return (partition_view_t) {
        .left = (view_t) { view.begin, item },
        .right = (view_t) { item + element_size, view.end },
        .delimiter = item
      };
    }
    item += element_size;
  }
  return (partition_view_t) { view, view_empty, NULL };
}

partition_span_t span_partition(
  span_t span, const void* del, compare_nosize_fn compare, index_t element_size
) {
  partition_view_t ret = view_partition(span.view, del, compare, element_size);
  return *((partition_span_t*)&ret);
}

////////////////////////////////////////////////////////////////////////////////

bool view_eq(view_t lhs, view_t rhs) {
  index_t size = view_size_bytes(lhs);
  if (size != view_size_bytes(rhs)) return false;
  return memcmp(lhs.begin, rhs.begin, size) == 0;
}

bool span_eq(span_t lhs, span_t rhs) {
  return view_eq(lhs.view, rhs.view);
}

bool view_eq_deep(
  view_t lhs, view_t rhs, index_t element_size, compare_nosize_fn compare
) {
  index_t size = view_size(lhs, element_size);
  if (size != view_size(rhs, element_size)) return false;
  const byte* p_lhs = lhs.begin;
  const byte* p_rhs = rhs.begin;
  for (index_t i = 0; i < size; ++i) {
    if (compare(p_lhs, p_rhs) != 0) {
      return false;
    }
    p_lhs += element_size;
    p_rhs += element_size;
  }
  return true;
}

bool span_eq_deep(
  span_t lhs, span_t rhs, index_t element_size, compare_nosize_fn compare
) {
  return view_eq_deep(lhs.view, rhs.view, element_size, compare);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <alloca.h>
#endif

static inline void mem_swap(byte* lhs, byte* rhs, index_t size) {
  byte* tmp = alloca(size);
  memcpy(tmp, lhs, (size_t)size);
  memcpy(lhs, rhs, (size_t)size);
  memcpy(rhs, tmp, (size_t)size);
}

void span_set_bytes(span_t span, byte b) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  size_t size = end - begin;
  memset(begin, b, size);
}

void span_sort(span_t span, index_t element_size, compare_nosize_fn cmp) {
  index_t length = span_size(span, element_size);
  qsort(span.begin, length, element_size, cmp);
}

void span_reverse(span_t span, index_t element_size) {
  if (span_is_empty(span)) return;
  byte* begin = span.begin;
  byte* end = (byte*)span.end - element_size;
  while (begin < end) {
    mem_swap(begin, end, element_size);
    begin += element_size;
    end -= element_size;
  }
  
}

void span_rotate(span_t span, index_t count, index_t element_size) {
  if (span_is_empty(span)) return;
  index_t size = span_size(span, element_size);
  if (count < 0) {
    count += size;
    if (count < 0) count = -count;
  }
  count %= size;
  span_reverse(span, element_size); // replace with memrev?
  pair_span_t split = span_split(span, count, element_size);
  span_reverse(split.left, element_size);
  span_reverse(split.right, element_size);
}

void span_shuffle(span_t span, index_t element_size) {
  if (span_is_empty(span)) return;
  index_t size = span_size(span, element_size);
  byte* item = span.begin;
  byte* const end = span.end;
  while (item < end) {
    index_t index = rand() % size;
    byte* swap = span_ref(span, index, element_size);
    if (item != swap) mem_swap(item, swap, element_size);
    item += element_size;
  }
}

void span_swap(span_t span, index_t idx1, index_t idx2, index_t element_size) {
  if (span_is_empty(span)) return;
  byte* lhs = span_ref(span, idx1, element_size);
  byte* rhs = span_ref(span, idx2, element_size);
  if (lhs != rhs) mem_swap(lhs, rhs, element_size);
}

void span_swap_back(span_t span, index_t index, index_t element_size) {
  if (span_is_empty(span)) return;
  byte* value = span_ref(span, index, element_size);
  byte* last = (byte*)span.end - element_size;
  if (value != last) mem_swap(value, last, element_size);
}

////////////////////////////////////////////////////////////////////////////////

#include "slice.h"
#include "span_byte.h"

slice_t span_byte_to_slice(span_byte_t span) {
  assert(span.end >= span.begin);
  slice_t ret = { .begin = (char*)span.begin, .size = (span.end - span.begin) };
  // If the span contains a null-terminator, remove it for the slice.
  // This will happen when using the SLICE macro on a string literal.
  if (span.end > span.begin && *(span.end - 1) == '\0') {
    ret.size -= 1;
  }
  return ret;
}

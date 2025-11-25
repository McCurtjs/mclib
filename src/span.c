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

#include "view.h"
#include "span.h"

#include <stdlib.h> // qsort, rand
#include <memory.h> // memcpy, memset

const span_t span_empty = { .begin = NULL, .end = NULL };

bool span_read(span_t span, index_t index, void* out, index_t element_size) {
  return view_read(span.view, index, out, element_size);
}

bool span_read_front(span_t span, void* out, index_t element_size) {
  return view_read_front(span.view, out, element_size);
}

bool span_read_back(span_t span, void* out, index_t element_size) {
  return view_read_back(span.view, out, element_size);
}

void span_write(span_t span, index_t index, const void* item, index_t el_size) {
  assert(item);
  index_t size = span_size(span, el_size);
  if (index < 0) index += size;
  assert(index >= 0 && index < size);
  memcpy((byte*)span.begin + index * el_size, item, el_size);
}

////////////////////////////////////////////////////////////////////////////////

bool span_eq(span_t lhs, span_t rhs) {
  return view_eq(lhs.view, rhs.view);
}

bool span_eq_deep(
  span_t lhs, span_t rhs, compare_nosize_fn compare, index_t element_size
) {
  return view_eq_deep(lhs.view, rhs.view, compare, element_size);
}

bool span_is_ordered(span_t span, compare_nosize_fn cmp, index_t el_size) {
  return view_is_ordered(span.view, cmp, el_size);
}

////////////////////////////////////////////////////////////////////////////////

span_t span_subspan(span_t span, index_t start, index_t end, index_t el_size) {
  view_t ret = view_subview(span.view, start, end, el_size);
  return *((span_t*)&ret);
}

span_t span_drop(span_t span, index_t count, index_t element_size) {
  view_t ret = view_drop(span.view, count, element_size);
  return *((span_t*)&ret);
}

span_t span_take(span_t span, index_t count, index_t element_size) {
  view_t ret = view_take(span.view, count, element_size);
  return *((span_t*)&ret);
}

pair_span_t span_split(span_t span, index_t element_size) {
  pair_view_t ret = view_split(span.view, element_size);
  return *((pair_span_t*)&ret);
}

pair_span_t span_split_at(span_t span, index_t pivot, index_t element_size) {
  pair_view_t ret = view_split_at(span.view, pivot, element_size);
  return *((pair_span_t*)&ret);
}

partition_span_t span_partition(
  span_t span, const void* del, compare_nosize_fn compare, index_t element_size
) {
  partition_view_t ret = view_partition(span.view, del, compare, element_size);
  return *((partition_span_t*)&ret);
}

partition_span_t span_partition_at(span_t span, index_t index, index_t elsize) {
  partition_view_t ret = view_partition_at(span.view, index, elsize);
  return *((partition_span_t*)&ret);
}

partition_span_t span_partition_match(
  span_t span, predicate_fn matcher, index_t element_size
) {
  partition_view_t ret = view_partition_match(span.view, matcher, element_size);
  return *((partition_span_t*)&ret);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef __WASM__
# include <alloca.h>
#else
# include <malloc.h>
#endif

#ifdef _MSC_VER
# define alloca _alloca
//# define alloca _malloca // consider using this for null-returning over crash?
#endif

static inline void mem_swap(byte* lhs, byte* rhs, index_t element_size) {
  assert(element_size > 0);
  assert(lhs);
  assert(rhs);
  byte* tmp = alloca(element_size);
  memcpy(tmp, lhs, (size_t)element_size);
  memcpy(lhs, rhs, (size_t)element_size);
  memcpy(rhs, tmp, (size_t)element_size);
}

void span_set_bytes(span_t span, byte b) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  size_t size = end - begin;
  memset(begin, b, size);
}

void span_fill(span_t span, const void* value, index_t element_size) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(value);
  assert(begin <= end);
  while (begin < end) {
    memcpy(begin, value, element_size);
  }
}

void span_sort(span_t span, compare_nosize_fn cmp, index_t element_size) {
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

void span_reverse_bytes(span_t span) {
  if (span_is_empty(span)) return;
  byte* begin = span.begin;
  byte* end = (byte*)span.end - 1;
  byte tmp;
  while (begin < end) {
    tmp = *begin;
    *begin++ = *end;
    *end-- = tmp;
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
  span_reverse_bytes(span); // replace with memrev?
  pair_span_t split = span_split_at(span, count, element_size);
  span_reverse_bytes(split.left);
  span_reverse_bytes(split.right);
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

void ispan_copy_range(span_t dst, view_t src, index_t index, index_t el_size) {
  SPAN_VALID(dst);
  VIEW_VALID(src);
  index_t dst_size = view_size(dst.view, el_size);
  index_t src_size = view_size(src, el_size);
  if (index >= dst_size) return;
  index_t to_copy = dst_size - index;
  if (to_copy > src_size) to_copy = src_size;
  char* dst_begin = dst.begin;
  memcpy(dst_begin + index * el_size, src.begin, to_copy * el_size);
}

span_t span_filter_inplace(
  span_t span, predicate_fn filter, index_t element_size
) {
  if (span_is_empty(span)) return span;
  byte* test = span.begin;
  byte* last = span.begin;
  while (test < (byte*)span.end) {
    if (filter(test)) {
      mem_swap(last, test, element_size);
      last += element_size;
    }
    test += element_size;
  }
  return (span_t) { .begin = span.begin, .end = last };
}

////////////////////////////////////////////////////////////////////////////////

index_t span_match_index(
  span_t span, predicate_fn matcher, index_t element_size
) {
  return view_match_index(span.view, matcher, element_size);
}

void* span_match_ref(
  span_t span, predicate_fn matcher, index_t element_size
) {
  return (void*)view_match_ref(span.view, matcher, element_size);
}

bool span_match(
  span_t span, predicate_fn matcher, void* out_value, index_t element_size
) {
  return view_match(span.view, matcher, out_value, element_size);
}

bool span_match_contains(
  span_t span, predicate_fn matcher, index_t element_size
) {
  return view_match_ref(span.view, matcher, element_size) != NULL;
}

index_t span_find_index(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return view_find_index(span.view, item, cmp, element_size);
}

void* span_find_ref(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return (void*)view_find_ref(span.view, item, cmp, element_size);
}

bool span_find(
  span_t span, const void* item, void* out_value,
  compare_nosize_fn cmp, index_t element_size
) {
  return view_find(span.view, item, out_value, cmp, element_size);
}

bool span_contains(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return view_find_ref(span.view, item, cmp, element_size) != NULL;
}

index_t span_search_index(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return view_search_index(span.view, item, cmp, element_size);
}

void* span_search_ref(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return (void*)view_search_ref(span.view, item, cmp, element_size);
}

bool span_search(
  span_t span, const void* item, void* out_found,
  compare_nosize_fn cmp, index_t element_size
) {
  return view_search(span.view, item, out_found, cmp, element_size);
}

bool span_search_contains(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return view_search_ref(span.view, item, cmp, element_size) != NULL;
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

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

#include <memory.h> // memcmp

const view_t view_empty = { .begin = NULL, .end = NULL };

const void* view_ref(view_t view, index_t index, index_t element_size) {
  if (view_is_empty(view)) return NULL;
  index_t size = view_size(view, element_size);
  if (index < 0) index = size + index;
  if (index < 0 || index >= size) return NULL;
  return (const byte*)view.begin + index * element_size;
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

view_t view_drop(view_t view, index_t count, index_t element_size) {
  if (view_is_empty(view)) return view;
  index_t size = view_size(view, element_size);
  const byte* begin = view.begin;
  const byte* end = view.end;
  if (count >= 0) {
    if (count >= size) begin = end;
    else begin += count * element_size;
  }
  else {
    if (-count >= size) end = begin;
    else end += count * element_size;
  }
  return (view_t) { .begin = begin, .end = end };
}

view_t view_take(view_t view, index_t count, index_t element_size) {
  if (view_is_empty(view)) return view;
  index_t size = view_size(view, element_size);
  const byte* begin = view.begin;
  const byte* end = view.end;
  if (count >= 0) {
    if (count >= size) return view;
    end = begin + count * element_size;
  }
  else {
    if (-count >= size) return view;
    begin = end + count * element_size;
  }
  return (view_t) { .begin = begin, .end = end };
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

partition_view_t view_partition(
  view_t view, const void* del, compare_nosize_fn compare, index_t element_size
) {
  if (view_is_empty(view)) return (partition_view_t) { view, view, NULL };
  const byte* item = view.begin;
  while (item < (const byte*)view.end) {
    if (!compare(item, del)) {
      return (partition_view_t) {
        .left = (view_t){ view.begin, item },
        .right = (view_t){ item + element_size, view.end },
        .delimiter = item
      };
    }
    item += element_size;
  }
  return (partition_view_t) { view, view_empty, NULL };
}

////////////////////////////////////////////////////////////////////////////////

bool view_eq(view_t lhs, view_t rhs) {
  index_t size = view_size_bytes(lhs);
  if (size != view_size_bytes(rhs)) return false;
  return memcmp(lhs.begin, rhs.begin, size) == 0;
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

////////////////////////////////////////////////////////////////////////////////

const void* view_select(view_t view, predicate_fn matcher, index_t elem_size) {
  VIEW_VALID(view);
  assert(matcher);
  const byte* seeker = view.begin;
  for (index_t i = 0; seeker < (const byte*)view.end; ++i) {
    if (matcher(seeker)) return seeker;
    seeker += elem_size;
  }
  return NULL;
}

index_t view_index_of(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
) {
  const byte* found = view_find(view, item, element_size, cmp);
  if (!found) return view_size(view, element_size);
  return (found - (const byte*)view.begin) / element_size;
}

const void* view_find(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
) {
  VIEW_VALID(view);
  assert(cmp);
  const byte* seeker = view.begin;
  for (index_t i = 0; seeker < (const byte*)view.end; ++i) {
    if (!cmp(seeker, item)) return seeker;
    seeker += element_size;
  }
  return NULL;
}

#include <stdlib.h>

index_t view_index_of_ordered(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
) {
  const byte* found = view_search(view, item, element_size, cmp);
  if (!found) return view_size(view, element_size);
  return (found - (const byte*)view.begin) / element_size;
}

const void* view_search(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
) {
  VIEW_VALID(view);
  assert(cmp);
  size_t size = (size_t)view_size(view, element_size);
  return bsearch(item, view.begin, size, element_size, cmp);
}

////////////////////////////////////////////////////////////////////////////////

#include "slice.h"
#include "span_byte.h"

slice_t view_to_slice(view_t view) {
  VIEW_VALID(view);
  slice_t ret = {
    .begin = (char*)view.begin,
    .size = ((char*)view.end - (char*)view.begin)
  };
  // If the view contains a null-terminator, remove it for the slice.
  // This will happen when using the SLICE macro on a string literal.
  if (view.end > view.begin && *((byte*)view.end - 1) == '\0') {
    ret.size -= 1;
  }
  return ret;
}

slice_t view_byte_to_slice(view_byte_t view) {
  return view_to_slice(view.base);
}

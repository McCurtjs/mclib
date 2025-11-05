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

// creates a pair with the left side containing the full view, and the right
//    side containing an empty view defined by the view's terminal to maintain
//    correct ordering.
#define _left_pair(VIEW) (pair_view_t) { VIEW, (view_t){ VIEW.end, VIEW.end } }

#define _right_pair(V) (pair_view_t) { (view_t){ V.begin, V.begin }, V }

bool view_read(view_t view, index_t index, void* out, index_t element_size) {
  assert(out);
  const void* ref = view_ref(view, index, element_size);
  if (!ref) return false;
  memcpy(out, ref, element_size);
  return true;
}

bool view_read_front(view_t view, void* out, index_t element_size) {
  assert(out);
  const void* ref = view_ref_front(view);
  if (!ref) return false;
  memcpy(out, ref, element_size);
  return true;
}

bool view_read_back(view_t view, void* out, index_t element_size) {
  assert(out);
  const void* ref = view_ref_back(view, element_size);
  if (!ref) return false;
  memcpy(out, ref, element_size);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool view_eq(view_t lhs, view_t rhs) {
  index_t size = view_size_bytes(lhs);
  if (size != view_size_bytes(rhs)) return false;
  return memcmp(lhs.begin, rhs.begin, size) == 0;
}

bool view_eq_deep(
  view_t lhs, view_t rhs, compare_nosize_fn compare, index_t element_size
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

bool view_is_ordered(view_t view, compare_nosize_fn cmp, index_t el_size) {
  index_t size = view_size(view, el_size);
  if (size < 2) return true;
  const byte* lhs = view.begin;
  const byte* rhs = (const byte*)view.end + el_size;
  while (rhs < (const byte*)view.end) {
    if (!cmp(lhs, rhs)) return false;
    lhs += el_size;
    rhs += el_size;
  }
  return true;
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

pair_view_t view_split(view_t view, index_t element_size) {
  if (view_is_empty(view)) return (pair_view_t) { view, view };
  index_t size = view_size(view, element_size);
  index_t pivot = size / 2;
  const byte* middle = (const byte*)view.begin + pivot * element_size;
  return (pair_view_t) {
    .left = (view_t){ view.begin, middle },
    .right = (view_t){ middle, view.end }
  };
}

pair_view_t view_split_at(view_t view, index_t pivot, index_t element_size) {
  if (view_is_empty(view)) return (pair_view_t) { view, view };
  index_t size = view_size(view, element_size);
  if (pivot >= size) return _left_pair(view);
  if (pivot < 0) pivot += size;
  if (pivot <= 0) return _right_pair(view);
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
  return (partition_view_t) { .pair = _left_pair(view), .delimiter = NULL };
}

partition_view_t view_partition_at(view_t view, index_t index, index_t elsize) {
  if (view_is_empty(view)) return (partition_view_t) { view, view, NULL };
  index_t size = view_size(view, elsize);
  if (index >= size) return (partition_view_t) { _left_pair(view), NULL };
  if (index < 0) index += size;
  if (index < 0) return (partition_view_t) { _right_pair(view), NULL };
  const byte* pivot = (const byte*)view.begin + index * elsize;
  return (partition_view_t) {
    .left = (view_t){ view.begin, pivot },
    .right = (view_t){ pivot + elsize, view.end },
    .delimiter = pivot
  };
}

partition_view_t view_partition_match(
  view_t view, predicate_fn matcher, index_t element_size
) {
  if (view_is_empty(view)) return (partition_view_t) { view, view, NULL };
  const byte* item = view.begin;
  while (item < (const byte*)view.end) {
    if (matcher(item)) {
      return (partition_view_t) {
        .left = (view_t){ view.begin, item },
        .right = (view_t){ item + element_size, view.end },
        .delimiter = item
      };
    }
  }
  return (partition_view_t) { .pair = _left_pair(view), .delimiter = NULL };
}

////////////////////////////////////////////////////////////////////////////////

index_t view_match_index(
  view_t view, predicate_fn matcher, index_t element_size
) {
  VIEW_VALID(view);
  assert(matcher);
  assert(element_size > 0);
  const byte* seeker = view.begin;
  index_t i;
  for (i = 0; seeker < (const byte*)view.end; ++i) {
    if (matcher(seeker)) return i;
    seeker += element_size;
  }
  return i;
}

const void* view_match_ref(
  view_t view, predicate_fn matcher, index_t element_size
) {
  VIEW_VALID(view);
  assert(matcher);
  assert(element_size > 0);
  const byte* seeker = view.begin;
  for (; seeker < (const byte*)view.end; seeker += element_size) {
    if (matcher(seeker)) {
      return seeker;
    }
  }
  return NULL;
}

bool view_match(
  view_t view, predicate_fn matcher, void* out_value, index_t element_size
) {
  const void* ref = view_match_ref(view, matcher, element_size);
  if (!ref) return false;
  memcpy(out_value, ref, element_size);
  return true;
}

bool view_match_contains(
  view_t view, predicate_fn matcher, index_t element_size
) {
  return view_match_ref(view, matcher, element_size) != NULL;
}

index_t view_find_index(
  view_t view, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  const byte* found = view_find_ref(view, item, cmp, element_size);
  if (!found) return view_size(view, element_size);
  return (found - (const byte*)view.begin) / element_size;
}

const void* view_find_ref(
  view_t view, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  VIEW_VALID(view);
  assert(cmp);
  assert(element_size > 0);
  const byte* seeker = view.begin;
  for (; seeker < (const byte*)view.end; seeker += element_size) {
    if (!cmp(seeker, item)) {
      return seeker;
    }
  }
  return NULL;
}

bool view_find(
  view_t view, const void* item, void* out_value,
  compare_nosize_fn cmp, index_t element_size
) {
  assert(out_value);
  const void* ref = view_find_ref(view, item, cmp, element_size);
  if (!ref) return false;
  memcpy(out_value, ref, element_size);
  return true;
}

bool view_contains(
  view_t view, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return view_find_ref(view, item, cmp, element_size) != NULL;
}

#include <stdlib.h>

index_t view_search_index(
  view_t view, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  const byte* found = view_search_ref(view, item, cmp, element_size);
  if (!found) return view_size(view, element_size);
  return (found - (const byte*)view.begin) / element_size;
}

const void* view_search_ref(
  view_t view, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  VIEW_VALID(view);
  assert(cmp);
  assert(element_size > 0);
  size_t size = (size_t)view_size(view, element_size);
  return bsearch(item, view.begin, size, element_size, cmp);
}

bool view_search(
  view_t view, const void* item, void* out_found,
  compare_nosize_fn cmp, index_t element_size
) {
  assert(out_found);
  const void* ref = view_search_ref(view, item, cmp, element_size);
  if (!ref) return false;
  memcpy(out_found, ref, element_size);
  return true;
}

bool view_search_contains(
  view_t view, const void* item, compare_nosize_fn cmp, index_t element_size
) {
  return view_search_ref(view, item, cmp, element_size) != NULL;
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

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

#ifndef MCLIB_VIEW_H_
#define MCLIB_VIEW_H_

//
// Container View
//
// A view is a read-only non-owning window into a contiguous block of memory,
// which can represent the contents of another structure such as an Array. a
// span, or a basic C-style array.

#include "types.h"
#include "span_base.h"

typedef struct pair_view_t {
  union {
    view_t begin[2];
    struct {
      view_t left;
      view_t right;
    };
  };
} pair_view_t;

typedef struct partition_view_t {
  union {
    pair_view_t pair;
    struct {
      view_t left;
      view_t right;
    };
  };
  const void* delimiter;
} partition_view_t;

////////////////////////////////////////////////////////////////////////////////
// Sub-ranges
////////////////////////////////////////////////////////////////////////////////

view_t view_subview(view_t view, index_t start, index_t end, index_t el_size);
view_t view_drop(view_t view, index_t count, index_t element_size);
view_t view_take(view_t view, index_t count, index_t element_size);
pair_view_t view_split(view_t view, index_t pivot, index_t element_size);
partition_view_t view_partition(
  view_t view, const void* del, compare_nosize_fn compare, index_t element_size
);

////////////////////////////////////////////////////////////////////////////////
// Equality testing
////////////////////////////////////////////////////////////////////////////////

bool view_eq(view_t lhs, view_t rhs);
bool view_eq_deep(view_t lh, view_t rh, index_t elsz, compare_nosize_fn cmp);

////////////////////////////////////////////////////////////////////////////////
// Algorithm
////////////////////////////////////////////////////////////////////////////////

// linear select
const void* view_select(view_t view, predicate_fn matcher, index_t elem_size);

// linear search
index_t view_index_of(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
);

// linear search
const void* view_find(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
);

// binary search
index_t view_index_of_ordered(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
);

// binary search
const void* view_search(
  view_t view, const void* item, index_t element_size, compare_nosize_fn cmp
);

#endif

////////////////////////////////////////////////////////////////////////////////
// Templatized specialization
////////////////////////////////////////////////////////////////////////////////

#ifdef con_type

#ifdef con_prefix
# define _con_name con_prefix
#else
# define _con_name con_type
#endif

#define _view_type MACRO_CONCAT3(view_, _con_name, _t)
#define _prefix(_fn) MACRO_CONCAT3(view_, _con_name, _fn)

typedef struct _view_type {
  union {
    view_t base;
    struct {
      const con_type* begin;
      const con_type* end;
    };
  };
} _view_type;

static inline _view_type MACRO_CONCAT(view_, _con_name)
(const con_type* begin, const con_type* end) {
  assert(begin <= end);
  return (_view_type) { begin, end };
}

static inline _view_type _prefix(_cast)
(view_t view) {
  return *(_view_type*)&view;
}

static inline index_t _prefix(_size)
(_view_type view) {
  return view_size(view.base, sizeof(con_type));
}

static inline index_t _prefix(_size_bytes)
(_view_type view) {
  return view_size_bytes(view.base);
}

static inline bool _prefix(_eq)
(_view_type lhs, _view_type rhs) {
  return view_eq(lhs.base, rhs.base);
}

static inline bool _prefix(_is_empty)
(_view_type view) {
  return view_is_empty(view.base);
}

static inline const con_type* _prefix(_ref)
(_view_type view, index_t index) {
  return view_ref(view.base, index, sizeof(con_type));
}

static inline const con_type* _prefix(_ref_front)
(_view_type view) {
  return view_ref_front(view.base);
}

static inline const con_type* _prefix(_ref_back)
(_view_type view) {
  return view_ref_back(view.base, sizeof(con_type));
}

static inline con_type _prefix(_get)
(_view_type view, index_t index) {
  const con_type* ret = view_ref(view.base, index, sizeof(con_type));
  assert(ret);
  return *ret;
}

static inline con_type _prefix(_get_front)
(_view_type view) {
  const con_type* ret = view_ref_front(view.base);
  assert(ret);
  return *ret;
}

static inline con_type _prefix(_get_back)
(_view_type view) {
  const con_type* ret = view_ref_back(view.base, sizeof(con_type));
  assert(ret);
  return *ret;
}

static inline _view_type _prefix(_drop)
(_view_type view, index_t count) {
  view.base = view_drop(view.base, count, sizeof(con_type));
  return view;
}

static inline _view_type _prefix(_take)
(_view_type view, index_t count) {
  view.base = view_take(view.base, count, sizeof(con_type));
  return view;
}

static inline _view_type _prefix(_subview)
(_view_type view, index_t start, index_t end) {
  view.base = view_subview(view.base, start, end, sizeof(con_type));
  return view;
}

static inline const con_type* _prefix(_select)
(_view_type view, predicate_fn matcher) {
  return view_select(view.base, matcher, sizeof(con_type));
}

#ifdef con_cmp

static inline bool _prefix(_eq_deep)
(_view_type lhs, _view_type rhs) {
  return view_eq_deep(lhs.base, rhs.base, sizeof(con_type), con_cmp);
}

static inline index_t _prefix(_index_of)
(_view_type view, const void* item) {
  return view_index_of(view.base, item, sizeof(con_type), con_cmp);
}

static inline const con_type* _prefix(_find)
(_view_type view, const void* item) {
  return view_find(view.base, item, sizeof(con_type), con_cmp);
}

static inline index_t _prefix(_index_of_ordered)
(_view_type view, const void* item) {
  return view_index_of_ordered(view.base, item, sizeof(con_type), con_cmp);
}

static inline const con_type* _prefix(_search)
(_view_type view, const void* item) {
  return view_search(view.base, item, sizeof(con_type), con_cmp);
}

#endif

#undef _con_name
#undef _view_type
#undef _prefix

#endif


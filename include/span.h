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

#ifndef MCLIB_SPAN_H_
#define MCLIB_SPAN_H_

//
// Container Span
//
// A span of another container or constant of contiguous data. The span does not
// typically "own" the data it contains, but can access and be used to modifiy
// it in place. A span cannot be grown dynamically, but similarly non-owning
// spans can be generated from contained subsets.
//
// In order to define type-specific continers, include or re-include the header
// after #defining con_type with the desired type, and optionally con_prefix to
// set the prefix type specifier (if not set, the type will be used directly).
//
// When defined, the following will be created (inline, with no overhead):
//
// #define con_type A
// #define con_prefix a
// #define con_cmp compare_fn // optional, reuqired for sort/search functions
// #include "span.h"
// #undef con_type
// #undef con_prefix
// #undef con_cmp
//
// // Create
// span_a_t span_a(span_a_t, A* begin, A* end);
//
// // Utility
// index_t  span_a_size(span_a_t);
// index_t  span_a_size_bytes(span_a_t);
// bool     span_a_empty(span_a_t);
//
// // Accessors
// A        span_a_get(span_a_t, index_t index);
// A        span_a_get_front(span_a_t, index_t index);
// A        span_a_get_back(span_a_t, index_t index);
// A*       span_a_ref(span_a_t, index_t index);
// A*       span_a_ref_front(span_a_t);
// A*       span_a_ref_back(span_a_t);
//
// // Subsets
// span_a_t span_a_drop_front(span_a_t);
// span_a_t span_a_drop_first(span_a_t, index_t count);
// span_a_t span_a_drop_back(span_a_t);
// span_a_t span_a_drop_last(span_a_t, index_t count);
// span_a_t span_a_subspan(span_a_t, index_t start, index_t end);
// span_a_t span_a_first(span_a_t, index_t count);
// span_a_t span_a_last(span_a_t, index_t count);
//
// // Algorithm
// void     span_a_reverse(span_a_t);
// void     span_a_set_bytes(span_a_t);
//

#include "types.h"
#include "span_base.h"

////////////////////////////////////////////////////////////////////////////////
// Return result tuples
////////////////////////////////////////////////////////////////////////////////

typedef struct pair_span_t {
  union {
    span_t begin[2];
    struct {
      span_t left;
      span_t right;
    };
  };
} pair_span_t;

typedef struct partition_span_t {
  union {
    pair_span_t pair;
    struct {
      span_t left;
      span_t right;
    };
  };
  void* delimiter;
} partition_span_t;

////////////////////////////////////////////////////////////////////////////////
// Sub-ranges
////////////////////////////////////////////////////////////////////////////////

span_t span_subspan(span_t span, index_t start, index_t end, index_t el_sz);
span_t span_drop(span_t span, index_t count, index_t element_size);
span_t span_take(span_t span, index_t count, index_t element_size);
pair_span_t span_split(span_t span, index_t pivot, index_t element_size);
partition_span_t span_partition(
  span_t span, const void* del, compare_nosize_fn compare, index_t element_size
);

////////////////////////////////////////////////////////////////////////////////
// Equality testing
////////////////////////////////////////////////////////////////////////////////

bool span_eq(span_t lhs, span_t rhs);
bool span_eq_deep(span_t lh, span_t rh, index_t elsz, compare_nosize_fn cmp);

////////////////////////////////////////////////////////////////////////////////
// Algorithm
////////////////////////////////////////////////////////////////////////////////

void span_set_bytes(span_t span, byte b);
void span_reverse_bytes(span_t span);
void span_sort(span_t span, index_t element_size, compare_nosize_fn cmp);
void span_reverse(span_t span, index_t element_size);
void span_rotate(span_t span, index_t count, index_t element_size);
void span_shuffle(span_t span, index_t element_size);
void span_swap(span_t span, index_t idx1, index_t idx2, index_t element_size);
void span_swap_back(span_t span, index_t index, index_t element_size);

#define span_copy_range(span_dst, view_src, index, element_size) \
  ispan_copy_range(span_dst, _s2v(view_src), index, element_size)

void ispan_copy_range(span_t dst, view_t src, index_t index, index_t el_size);

// linear selection
void* span_select(span_t span, predicate_fn matcher, index_t element_size);

// linear search
index_t span_index_of(
  span_t span, const void* item, index_t element_size, compare_nosize_fn cmp
);

// linear search
void* span_find(
  span_t span, const void* item, index_t element_size, compare_nosize_fn cmp
);

// binary search
index_t span_index_of_ordered(
  span_t span, const void* item, index_t element_size, compare_nosize_fn cmp
);

// binary search
void* span_search(
  span_t span, const void* item, index_t element_size, compare_nosize_fn cmp
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

#define _span_type MACRO_CONCAT3(span_, _con_name, _t)
#define _prefix(_fn) MACRO_CONCAT3(span_, _con_name, _fn)

typedef struct _span_type {
  union {
    span_t base;
#ifdef con_view_type
    con_view_type view;
#endif
    struct {
      con_type* begin;
      con_type* end;
    };
  };
} _span_type;

static inline _span_type MACRO_CONCAT(span_, _con_name)
(con_type* begin, con_type* end) {
  assert(begin <= end);
  return (_span_type) { begin, end };
}

static inline _span_type _prefix(_cast)
(span_t span) {
  return *(_span_type*)&span;
}

static inline index_t _prefix(_size)
(_span_type span) {
  return span_size(span.base, sizeof(con_type));
}

static inline index_t _prefix(_size_bytes)
(_span_type span) {
  return span_size_bytes(span.base);
}

static inline bool _prefix(_eq)
(_span_type lhs, _span_type rhs) {
  return span_eq(lhs.base, rhs.base);
}

static inline bool _prefix(_empty)
(_span_type span) {
  return span_is_empty(span.base);
}

static inline con_type* _prefix(_ref)
(_span_type span, index_t index) {
  return span_ref(span.base, index, sizeof(con_type));
}

static inline con_type* _prefix(_ref_front)
(_span_type span) {
  return span_ref_front(span.base);
}

static inline con_type* _prefix(_ref_back)
(_span_type span) {
  return span_ref_back(span.base, sizeof(con_type));
}

static inline con_type _prefix(_get)
(_span_type span, index_t index) {
  con_type* ret = span_ref(span.base, index, sizeof(con_type));
  assert(ret);
  return *ret;
}

static inline con_type _prefix(_get_front)
(_span_type span) {
  con_type* ret = span_ref_front(span.base);
  assert(ret);
  return *ret;
}

static inline con_type _prefix(_get_back)
(_span_type span) {
  con_type* ret = span_ref_back(span.base, sizeof(con_type));
  assert(ret);
  return *ret;
}

static inline _span_type _prefix(_drop)
(_span_type span, index_t count) {
  span.base = span_drop(span.base, count, sizeof(con_type));
  return span;
}

static inline _span_type _prefix(_take)
(_span_type span, index_t count) {
  span.base = span_take(span.base, count, sizeof(con_type));
  return span;
}

static inline _span_type _prefix(_subspan)
(_span_type span, index_t start, index_t end) {
  span.base = span_subspan(span.base, start, end, sizeof(con_type));
  return span;
}

// Algorithm

static inline void _prefix(_set_bytes)
(_span_type span, byte b) {
  span_set_bytes(span.base, b);
}

static inline void _prefix(_reverse_bytes)
(_span_type span) {
  span_reverse_bytes(span.base);
}

static inline void _prefix(_reverse)
(_span_type span) {
  span_reverse(span.base, sizeof(con_type));
}

static inline void _prefix(_rotate)
(_span_type span, index_t count) {
  span_rotate(span.base, count, sizeof(con_type));
}

static inline void _prefix(_shuffle)
(_span_type span) {
  span_shuffle(span.base, sizeof(con_type));
}

static inline void _prefix(_swap)
(_span_type span, index_t idx1, index_t idx2) {
  span_swap(span.base, idx1, idx2, sizeof(con_type));
}

static inline void _prefix(_swap_back)
(_span_type span, index_t index) {
  span_swap_back(span.base, index, sizeof(con_type));
}

#ifdef con_view_type

static inline void _prefix(_copy_range)
(_span_type dst, con_view_type src, index_t index) {
  ispan_copy_range(dst.base, src.base, index, sizeof(con_type));
}

#endif

static inline con_type* _prefix(_select)
(_span_type span, predicate_fn matcher) {
  return span_select(span.base, matcher, sizeof(con_type));
}

#ifdef con_cmp

static inline bool _prefix(_eq_deep)
(_span_type lhs, _span_type rhs) {
  return span_eq_deep(lhs.base, rhs.base, sizeof(con_type), con_cmp);
}

static inline void _prefix(_sort)
(_span_type span) {
  span_sort(span.base, sizeof(con_type), con_cmp);
}

static inline index_t _prefix(_index_of)
(_span_type span, const void* item) {
  return span_index_of(span.base, item, sizeof(con_type), con_cmp);
}

static inline con_type* _prefix(_find)
(_span_type span, const void* item) {
  return span_find(span.base, item, sizeof(con_type), con_cmp);
}

static inline index_t _prefix(_index_of_ordered)
(_span_type span, const void* item) {
  return span_index_of_ordered(span.base, item, sizeof(con_type), con_cmp);
}

static inline con_type* _prefix(_search)
(_span_type span, const void* item) {
  return span_search(span.base, item, sizeof(con_type), con_cmp);
}

#endif

#undef _con_name
#undef _span_type
#undef _view_type
#undef _prefix

#endif

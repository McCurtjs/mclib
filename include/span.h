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
// span_a_t span_a_build(A* begin, A* end);
// span_a_t span_a_range(A* begin, index_t size);
// span_a_t span_a(A* begin, index_t size);
// span_a_t span_a_cast(span_t);
//
// // Utility
// index_t  span_a_size(span_a_t);
// index_t  span_a_size_bytes(span_a_t);
// bool     span_a_is_empty(span_a_t);
// bool     span_a_eq(span_a_t lhs, span_a_t rhs);
// bool     span_a_eq_deep(span_a_t lhs, span_a_t rhs);
// bool     span_a_is_ordered(span_a_t);
//
// // Accessors
// A*       span_a_ref(span_a_t, index_t index);
// A*       span_a_ref_front(span_a_t);
// A*       span_a_ref_back(span_a_t);
//
// bool     span_a_read(span_a_t, index_t, A* out);
// bool     span_a_read_front(span_a_t,  A* out);
// bool     span_a_read_back(span_a_t, A* out);
// 
// void     span_a_write(span_a_t, index_t index, const A* item);
//
// A        span_a_get(span_a_t, index_t index);
// A        span_a_get_front(span_a_t, index_t index);
// A        span_a_get_back(span_a_t, index_t index);
//
// // Subsets
// span_a_t span_a_subspan(span_a_t, index_t start, index_t end);
// span_a_t span_a_drop(span_a_t, index_t count);
// span_a_t span_a_take(span_a_t, index_t count);
// span_a_t span_a_split(span_a_t, index_t pivot);
// span_a_t span_a_partition(span_a_t, const A* del);
//
// // Algorithm
// index_t  span_a_match_index(span_a_t, predicate_fn);
// A*       span_a_match_ref(vuew_a_t, predicate_fn);
// bool     span_a_match(span_a_t, predicate_fn, A* out);
// bool     span_a_match_contains(span_a_t, predicate_fn);
//
// index_t  span_a_find_index(span_a_t, const A* item);
// A*       span_a_find_ref(span_a_t, const A* item);
// bool     span_a_find(span_a_t, const A* item, A* out);
// bool     span_a_contains(span_a_t, const A* item);
//
// index_t  span_a_search_index(span_a_t, const A* item);
// A*       span_a_search_ref(span_a_t, const A* item);
// bool     span_a_search(span_a_t, const A* item, A* out);
// bool     span_a_search_contains(span_a_t, const A* item);
//
// void     span_a_set_bytes(span_a_t);
// void     span_a_copy_range(span_a_t dst, view_a_t src, index_t pos);
// void     span_a_reverse(span_a_t);
// void     span_a_sort(span_a_t);
// void     span_a_rotate(span_a_t, index_t count);
// void     span_a_shuffle(span_a_t);
// void     span_a_swap(span_a_t, index_t id1, index_t id2);
// void     span_a_swap_back(span_a_t, index_t);
// void     span_a_filter_inplace(span_a_t, predicate_fn);
//

#include "types.h"
#include "span_base.h"

// Create pair_span_t and partition_span_t
#define tuple_type span_t
#include "pair.h"
#define delim_type void*
#define tuple_pair_type pair_span_t
#include "partition.h"
#undef tuple_pair_type
#undef delim_type
#undef tuple_type

////////////////////////////////////////////////////////////////////////////////
// Builders
////////////////////////////////////////////////////////////////////////////////

static inline span_t span_build(void* begin, void* end) {
  assert(begin <= end);
  return (span_t) { .begin = begin, .end = end };
}

static inline span_t span_range(void* begin, index_t size, index_t elsz) {
  assert(size >= 0);
  assert(elsz > 0);
  assert(begin || !size);
  return (span_t) { .begin = begin, .end = (byte*)begin + size * elsz };
}

////////////////////////////////////////////////////////////////////////////////
// Accessors
////////////////////////////////////////////////////////////////////////////////

index_t span_size_bytes(span_t span);
index_t span_size(span_t span, index_t element_size);
bool    span_is_empty(span_t span);

void*   span_ref(span_t span, index_t index, index_t element_size);
void*   span_ref_front(span_t span);
void*   span_ref_back(span_t span, index_t element_size);

bool    span_read(span_t span, index_t index, void* out, index_t element_size);
bool    span_read_front(span_t span, void* out, index_t element_size);
bool    span_read_back(span_t span, void* out, index_t element_size);

void    span_write(span_t span, index_t index, const void* item, index_t el_sz);

bool    span_eq(span_t lhs, span_t rhs);
bool    span_eq_deep(span_t lh, span_t rh, compare_nosize_fn, index_t elsz);
bool    span_is_ordered(span_t span, compare_nosize_fn, index_t element_size);

////////////////////////////////////////////////////////////////////////////////
// Sub-ranges
////////////////////////////////////////////////////////////////////////////////

span_t span_subspan(span_t span, index_t start, index_t end, index_t el_sz);
span_t span_drop(span_t span, index_t count, index_t element_size);
span_t span_take(span_t span, index_t count, index_t element_size);
pair_span_t span_split(span_t span, index_t element_size);
pair_span_t span_split_at(span_t span, index_t pivot, index_t element_size);
partition_span_t span_partition(
  span_t span, const void* del, compare_nosize_fn compare, index_t element_size
);
partition_span_t span_partition_at(span_t span, index_t index, index_t el_size);
partition_span_t span_partition_match(
  span_t span, predicate_fn matcher, index_t element_size
);

////////////////////////////////////////////////////////////////////////////////
// Algorithm
////////////////////////////////////////////////////////////////////////////////

void span_set_bytes(span_t span, byte b);
void span_fill(span_t span, const void* value, index_t element_size);
void span_reverse_bytes(span_t span);
void span_reverse(span_t span, index_t element_size);
void span_sort(span_t span, compare_nosize_fn cmp, index_t element_size);
void span_rotate(span_t span, index_t count, index_t element_size);
void span_shuffle(span_t span, index_t element_size);
void span_swap(span_t span, index_t idx1, index_t idx2, index_t element_size);
void span_swap_back(span_t span, index_t index, index_t element_size);

#define span_copy_range(span_dst, view_src, index, element_size) \
  ispan_copy_range(span_dst, _s2v(view_src), index, element_size)

void ispan_copy_range(span_t dst, view_t src, index_t index, index_t el_size);

span_t span_filter_inplace(span_t src, predicate_fn filter, index_t elem_size);

////////////////////////////////////////////////////////////////////////////////
// Algorithm-searching
////////////////////////////////////////////////////////////////////////////////

// linear predicate seek
index_t span_match_index(
  span_t span, predicate_fn matcher, index_t element_size
);
void* span_match_ref(
  span_t span, predicate_fn matcher, index_t element_size
);
bool span_match(
  span_t span, predicate_fn matcher, void* out_value, index_t element_size
);
bool span_match_contains(
  span_t span, predicate_fn matcher, index_t element_size
);

// linear search
index_t span_find_index(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
);
void* span_find_ref(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
);
bool span_find(
  span_t span, const void* item, void* out_value,
  compare_nosize_fn cmp, index_t element_size
);
bool span_contains(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
);

// binary search
index_t span_search_index(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
);
void* span_search_ref(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
);
bool span_search(
  span_t span, const void* item, void* out_value,
  compare_nosize_fn cmp, index_t element_size
);
bool span_search_contains(
  span_t span, const void* item, compare_nosize_fn cmp, index_t element_size
);

#endif

////////////////////////////////////////////////////////////////////////////////
// Templatized type specialization
////////////////////////////////////////////////////////////////////////////////

#if defined(con_type) && !defined(_con_void_only) && !defined(con_void_only)

#ifdef con_prefix
# define _con_name con_prefix
#else
# define _con_name con_type
#endif

#define _span_type MACRO_CONCAT3(span_, _con_name, _t)
#define _pair_type MACRO_CONCAT(pair_, _span_type)
#define _partition_type MACRO_CONCAT(partition_, _span_type)
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

#define tuple_type _span_type
#define tuple_pair_type _pair_type
#define tuple_partition_type _partition_type
#define delim_type con_type*
#include "pair.h"
#include "partition.h"
#undef tuple_type
#undef tuple_pair_type
#undef tuple_partition_type
#undef delim_type

#ifdef con_cmp
# define _con_cmp con_cmp
#elif !defined(con_no_cmp)
# define _con_cmp MACRO_CONCAT3(_span_, _con_name, _cmp)
extern int memcmp(const void* lhs, const void* rhs, size_t size);
static int _con_cmp(const void* lhs, const void* rhs) {
  if (lhs == rhs) return 0;
  if (!lhs || !rhs) return lhs ? 1 : -1;
  return memcmp(lhs, rhs, sizeof(con_type));
}
#endif

////////////////////////////////////////////////////////////////////////////////

static inline _span_type _prefix(_build)
(con_type* begin, con_type* end) {
  assert(begin <= end);
  return (_span_type) { .begin = begin, .end = end };
}

static inline _span_type _prefix(_range)
(con_type* begin, index_t size) {
  assert(size >= 0);
  assert(begin || !size);
  return (_span_type) { .begin = begin, .end = begin + size };
}

static inline _span_type MACRO_CONCAT(span_, _con_name)
(con_type* begin, index_t size) {
  return _prefix(_range)(begin, size);
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

static inline _span_type _prefix(_subspan)
(_span_type span, index_t start, index_t end) {
  span.base = span_subspan(span.base, start, end, sizeof(con_type));
  return span;
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

static inline _pair_type _prefix(_split)
(_span_type span) {
  pair_span_t pair = span_split(span.base, sizeof(con_type));
  return *(_pair_type*)&pair;
}

static inline _pair_type _prefix(_split_at)
(_span_type span, index_t pivot) {
  pair_span_t pair = span_split_at(span.base, pivot, sizeof(con_type));
  return *(_pair_type*)&pair;
}

static inline _partition_type _prefix(_partition_at)
(_span_type span, index_t index) {
  partition_span_t ret = span_partition_at(span.base, index, sizeof(con_type));
  return *(_partition_type*)&ret;
}

static inline _partition_type _prefix(_partition_match)
(_span_type span, predicate_fn matcher) {
  partition_span_t partition = span_partition_match(
    span.base, matcher, sizeof(con_type)
  );
  return *(_partition_type*)&partition;
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

static inline _span_type _prefix(_filter_inplace)
(_span_type span, predicate_fn filter) {
  span.base = span_filter_inplace(span.base, filter, sizeof(con_type));
  return span;
}

static inline index_t _prefix(_match_index)
(_span_type span, predicate_fn matcher) {
  return span_match_index(span.base, matcher, sizeof(con_type));
}

static inline con_type* _prefix(_match_ref)
(_span_type span, predicate_fn matcher) {
  return span_match_ref(span.base, matcher, sizeof(con_type));
}

static inline bool _prefix(_match)
(_span_type type, predicate_fn matcher, con_type* out_found) {
  return span_match(type.base, matcher, out_found, sizeof(con_type));
}

#ifdef _con_cmp

static inline bool _prefix(_eq_deep)
(_span_type lhs, _span_type rhs) {
  return span_eq_deep(lhs.base, rhs.base, _con_cmp, sizeof(con_type));
}

static inline bool _prefix(_is_ordered)
(_span_type span) {
  return span_is_ordered(span.base, _con_cmp, sizeof(con_type));
}

static inline _partition_type _prefix(_partition)
(_span_type span, const con_type* del) {
  partition_span_t part = span_partition(
    span.base, del, _con_cmp, sizeof(con_type)
  );
  return *(_partition_type*)&part;
}

static inline void _prefix(_sort)
(_span_type span) {
  span_sort(span.base, _con_cmp, sizeof(con_type));
}

static inline index_t _prefix(_find_index)
(_span_type span, const con_type* item) {
  return span_find_index(span.base, item, _con_cmp, sizeof(con_type));
}

static inline con_type* _prefix(_find_ref)
(_span_type span, const con_type* item) {
  return span_find_ref(span.base, item, _con_cmp, sizeof(con_type));
}

static inline bool _prefix(_find)
(_span_type span, const con_type* item, con_type* out_found) {
  return span_find(span.base, item, out_found, _con_cmp, sizeof(con_type));
}

static inline bool _prefix(_contains)
(_span_type span, const con_type* item) {
  return span_contains(span.base, item, _con_cmp, sizeof(con_type));
}

static inline index_t _prefix(_search_index)
(_span_type span, const con_type* item) {
  return span_search_index(span.base, item, _con_cmp, sizeof(con_type));
}

static inline con_type* _prefix(_search_ref)
(_span_type span, const con_type* item) {
  return span_search_ref(span.base, item, _con_cmp, sizeof(con_type));
}

static inline bool _prefix(_search)
(_span_type span, const con_type* item, con_type* out_found) {
  return span_search(span.base, item, out_found, _con_cmp, sizeof(con_type));
}

static inline bool _prefix(_search_contains)
(_span_type span, const con_type* item) {
  return span_search_contains(span.base, item, _con_cmp, sizeof(con_type));
}

#endif

#undef _con_name
#undef _con_cmp
#undef _span_type
#undef _pair_type
#undef _partition_type
#undef _prefix

#endif

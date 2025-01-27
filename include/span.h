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

#ifndef _MCLIB_SPAN_H_
#define _MCLIB_SPAN_H_

#include "types.h"

typedef struct span_t {
  void* begin;
  void* end;
} span_t;

static inline index_t ispan_size_bytes(span_t span) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  return end - begin;
}

static inline index_t ispan_size(span_t span, index_t element_size) {
  assert(element_size > 0);
  return ispan_size_bytes(span) / element_size;
}

/*
static inline void* ispan_ref(span_t span, index_t index) {
  if (index < 0) return NULL;
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  if (!begin || end <= begin) return NULL;
  byte* ret = begin + (index * span.element_size);
  if (ret >= end) return NULL;
  return ret;
}

static inline void* ispan_ref_front(span_t span) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  if (!begin || end <= begin) return NULL;
  return begin;
}

static inline void* ispan_ref_back(span_t span) {
  byte* end = span.end;
  if (!span.begin || span.end <= span.begin) return NULL;
  return end - span.element_size;
}

static inline span_t ispan_drop_front(span_t span) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  if (end - begin <= 0) return span;
  span.begin = begin + span.element_size;
  return span;
}

static inline span_t ispan_drop_back(span_t span) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  if (end - begin <= 0) return span;
  span.end = end - span.element_size;
  return span;
}

static inline span_t ispan_subspan(span_t span, index_t start, index_t finish) {
  byte* begin = span.begin;
  byte* end = span.end;
  index_t size = ispan_size(span);
  assert(begin <= end);
  if (begin == end) return span;
  if (start < 0) start = size + start;
  if (start < 0) start = 0;
  if (finish > size) finish = size;
  if (finish < 0) finish = size + finish;
  if (finish < start) finish = start;
  span.begin = begin + start * span.element_size;
  span.end = begin + finish * span.element_size;
  return span;
}

static inline span_t ispan_first(span_t span, index_t count) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  if (begin + count * span.element_size >= end) return span;
  span.end = begin + count * span.element_size;
  return span;
}

static inline span_t ispan_last(span_t span, index_t count) {
  byte* begin = span.begin;
  byte* end = span.end;
  assert(begin <= end);
  if (end - count * span.element_size < begin) return span;
  span.begin = end - count * span.element_size;
  return span;
}//*/

void ispan_set_bytes(span_t span, byte b);

typedef int (*compare_fn)(const void* a, const void* b);

bool ispan_eq(span_t a, span_t b);
bool ispan_eq_deep(span_t lhs, span_t rhs, index_t el_size, compare_fn cmp);

void ispan_sort(span_t span, index_t element_size, compare_fn cmp);

#define span_foreach(VAR, SPAN)                                               \
  VAR = SPAN.begin;                                                           \
  for (; (byte*)VAR < (byte*)SPAN.end; ++VAR)                                 //

#define span_foreach_index(VAR, INDEX, SPAN)                                  \
  VAR = SPAN.begin;                                                           \
  for (index_t INDEX = 0; (byte*)VAR < (byte*)SPAN.end; ++VAR, ++INDEX)       //

#endif

#ifdef con_type

#ifdef con_prefix
# define _full_prefix MACRO_CONCAT(span_, con_prefix)
#else
# define _full_prefix MACRO_CONCAT(span_, con_type)
#endif

#define _span_type MACRO_CONCAT(_full_prefix, _t)
#define _prefix(_fn) MACRO_CONCAT(_full_prefix, _fn)

typedef struct _span_type {
  con_type* begin;
  con_type* end;
} _span_type;

#define SPAN(S) { .begin = (S), .end = (S) + ARRAY_COUNT(S) }

static inline _span_type _full_prefix
(con_type* begin, con_type* end) {
  assert(begin <= end);
  return (_span_type) { .begin = begin, .end = end };
}

static inline _span_type _prefix(_cast)
(span_t span) {
  return *(_span_type*)&span;
}

static inline index_t _prefix(_size)
(_span_type span) {
  assert(span.begin <= span.end);
  return (span.end - span.begin);
}

static inline index_t _prefix(_size_bytes)
(_span_type span) {
  assert(span.begin <= span.end);
  return (span.end - span.begin) * sizeof(con_type);
}

static inline bool _prefix(_eq)
(_span_type lhs, _span_type rhs) {
  return ispan_eq(*(span_t*)&lhs, *(span_t*)&rhs);
}

static inline bool _prefix(_empty)
(_span_type span) {
  assert(span.begin <= span.end);
  return span.begin >= span.end;
}

static inline con_type* _prefix(_ref)
(_span_type span, index_t index) {
  if (index < 0) return NULL;
  assert(span.begin <= span.end);
  if (!span.begin || span.end <= span.begin) return NULL;
  con_type* ret = span.begin + index;
  if (ret >= span.end) return NULL;
  return ret;
}

static inline con_type* _prefix(_ref_front)
(_span_type span) {
  assert(span.begin <= span.end);
  if (!span.begin || span.end <= span.begin) return NULL;
  return span.begin;
}

static inline con_type* _prefix(_ref_back)
(_span_type span) {
  assert(span.begin <= span.end);
  if (!span.begin || span.end <= span.begin) return NULL;
  return span.end - 1;
}

static inline con_type _prefix(_get)
(_span_type span, index_t index) {
  con_type* ret = _prefix(_ref)(span, index);
  assert(ret);
  return *ret;
}

static inline con_type _prefix(_get_front)
(_span_type span) {
  con_type* ret = _prefix(_ref_front)(span);
  assert(ret);
  return *ret;
}

static inline con_type _prefix(_get_back)
(_span_type span) {
  con_type* ret = _prefix(_ref_back)(span);
  assert(ret);
  return *ret;
}

static inline _span_type _prefix(_drop_front)
(_span_type span) {
  assert(span.begin <= span.end);
  if (span.begin < span.end) ++span.begin;
  return span;
}

static inline _span_type _prefix(_drop_first)
(_span_type span, index_t count) {
  assert(span.begin <= span.end);
  if (span.begin + count >= span.end) span.begin = span.end;
  else span.begin += count;
  return span;
}

static inline _span_type _prefix(_drop_back)
(_span_type span) {
  assert(span.begin <= span.end);
  if (span.begin < span.end) --span.end;
  return span;
}

static inline _span_type _prefix(_drop_last)
(_span_type span, index_t count) {
  assert(span.begin <= span.end);
  if (span.begin + count >= span.end) span.end = span.begin;
  else span.end -= count;
  return span;
}

static inline _span_type _prefix(_subspan)
(_span_type span, index_t start, index_t end) {
  index_t size = _prefix(_size)(span);
  if (span.begin == span.end) return span;
  if (start < 0) start = size + start;
  if (start < 0) start = 0;
  if (end > size) end = size;
  if (end < 0) end = size + end;
  if (end < start) end = start;
  span.end = span.begin + end;
  span.begin += start;
  return span;
}

static inline _span_type _prefix(_first)
(_span_type span, index_t count) {
  assert(span.begin <= span.end);
  if (span.begin == NULL) return span;
  con_type* end = span.begin + count;
  if (end >= span.end) return span;
  span.end = end;
  return span;
}

static inline _span_type _prefix(_last)
(_span_type span, index_t count) {
  assert(span.begin <= span.end);
  if (span.begin == NULL) return span;
  con_type* begin = span.end - count;
  if (begin <= span.begin) return span;
  span.begin = begin;
  return span;
}

// Algorithm

static inline void _prefix(_reverse)
(_span_type span) {
  assert(span.begin <= span.end);
  if (span.end <= span.begin) return;
  --span.end;
  while (span.begin < span.end) {
    con_type temp = *span.begin;
    *span.begin++ = *span.end;
    *span.end-- = temp;
  }
}

static inline void _prefix(_set_bytes)
(_span_type span, byte b) {
  ispan_set_bytes(*(span_t*)&span, b);
}

#ifdef con_cmp

static inline bool _prefix(_eq_deep)
(_span_type lhs, _span_type rhs) {
  static int (*cmp)(const con_type* a, const con_type* b) = con_cmp;
  if (ispan_size_bytes(*(span_t*)&lhs) != ispan_size_bytes(*(span_t*)&rhs)) {
    return false;
  }
  con_type* right = rhs.begin;
  con_type* span_foreach(left, lhs) {
    if (cmp(left, right) == false) {
      return false;
    }
    ++right;
  }
  return true;
}

static inline void _prefix(_sort)
(_span_type span) {
  static int (*cmp)(const con_type* a, const con_type* b) = con_cmp;
  ispan_sort(*(span_t*)&span, sizeof(con_type), (compare_fn)cmp);
}

#endif

#undef _span_type
#undef _prefix
#undef _span_full_prefix
#undef _full_prefix

#endif

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

// \brief A view is a constant window into a contiguous array of data. The view
//    is non-owning and its contents can't be modified through the view itself.
//
// \brief see also: span_t
typedef struct view_t {
  const void* begin;
  const void* end;
} view_t;

// \brief A span is a window into a contiguous array of data. The span is non-
//    owning, but has permission to modify the contents of the underlying data.
//
// \brief While the values in a span can be modified, the span itself is unaware
//    of the source and can't allocate additional space or perform item
//    insertions or removals.
typedef struct span_t {
  union {
    view_t view;
    struct {
      void* begin;
      void* end;
    };
  };
} span_t;

extern const span_t span_empty;
extern const view_t view_empty;

#define SPAN(S) { .begin = (S), .end = (S) + ARRAY_COUNT(S) }

#define span_foreach(VAR, SPAN)                                               \
  VAR = SPAN.begin;                                                           \
  for (; (const byte*)VAR < (const byte*)SPAN.end; ++VAR)                     //

#define span_foreach_index(V, IDX, SPAN)                                      \
  V = SPAN.begin;                                                             \
  for (index_t IDX = 0; (const byte*)V < (const byte*)SPAN.end; ++V, ++IDX)   //

#define view_foreach span_foreach
#define view_foreach_index span_foreach_index

static inline view_t span_to_view(span_t span) { return span.view; }
static inline view_t view_to_view(view_t view) { return view; }

#define _s2v(VAL) _Generic((VAL),                                             \
  span_t: span_to_view, view_t: view_to_view                                  \
)(VAL)                                                                        //

#define SPAN_VALID(SPAN) do {                                                 \
  span_t test_span = (SPAN);                                                  \
  assert((byte*)test_span.begin <= (byte*)test_span.end);                     \
} while(false)                                                                //

#define VIEW_VALID(SPAN) do {                                                 \
  view_t test_span = (SPAN);                                                  \
  assert((const byte*)test_span.begin <= (const byte*)test_span.end);         \
} while(false)                                                                //

////////////////////////////////////////////////////////////////////////////////
// Size and element count
////////////////////////////////////////////////////////////////////////////////

static inline index_t view_size_bytes(view_t span) {
  const byte* begin = (const byte*)span.begin;
  const byte* end = (const byte*)span.end;
  assert(begin <= end);
  return end - begin;
}

static inline index_t span_size_bytes(span_t span) {
  return view_size_bytes(span.view);
}

static inline index_t view_size(view_t view, index_t element_size) {
  assert(element_size > 0);
  return view_size_bytes(view) / element_size;
}

static inline index_t span_size(span_t span, index_t element_size) {
  return view_size(span.view, element_size);
}

static inline bool view_is_empty(view_t view) {
  VIEW_VALID(view);
  return view.begin == view.end;
}

static inline bool span_is_empty(span_t span) {
  return view_is_empty(span.view);
}

////////////////////////////////////////////////////////////////////////////////
// Basic element access
////////////////////////////////////////////////////////////////////////////////

const void* view_ref(view_t view, index_t index, index_t element_size);
void* span_ref(span_t span, index_t index, index_t element_size);

static inline const void* view_ref_front(view_t view) {
  if (view_is_empty(view)) return NULL;
  return view.begin;
}

static inline const void* view_ref_back(view_t view, index_t element_size) {
  if (view_is_empty(view)) return NULL;
  const byte* ret = (const byte*)view.end - element_size;
  return ret;
}

static inline void* span_ref_front(span_t span) {
  if (span_is_empty(span)) return NULL;
  return span.begin;
}

static inline void* span_ref_back(span_t span, index_t element_size) {
  if (span_is_empty(span)) return NULL;
  byte* ret = (byte*)span.end - element_size;
  return ret;
}

#endif

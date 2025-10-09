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
#include "array.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
#endif

// internal opaque structure:
typedef struct Array_Internal {
  // public (read only)
  union {
    span_t span;
    struct {
      union { byte* begin; byte* data; };
      byte* end;
      index_t element_size;
    };
  };
  index_t capacity;
  index_t size;
  index_t size_bytes;

  // private
} Array_Internal;

#define DARRAY_STARTING_SIZE 2

#define GROWTH_FACTOR \
  MAX(DARRAY_STARTING_SIZE, a->capacity + a->capacity / 2)

#define DARRAY_INTERNAL \
  Array_Internal* a = (Array_Internal*)(a_in); \
  assert(a)

#define DARRAY_INTERNAL_CONST \
  const Array_Internal* a = (const Array_Internal*)(a_in); \
  assert(a)

Array iarr_new(index_t element_size) {
  Array_Internal* ret = malloc(sizeof(Array_Internal));
  assert(ret);
  *ret = (Array_Internal) {
    .begin = NULL,
    .end = NULL,
    .element_size = element_size,
    .capacity = 0,
    .size = 0,
    .size_bytes = 0,
  };
  return (Array)ret;
}

Array iarr_new_reserve(index_t element_size, index_t capacity) {
  Array_Internal* ret = (Array_Internal*)iarr_new(element_size);
  if (capacity <= 0) return (Array)ret;
  byte* mem = malloc(element_size * capacity);
  assert(mem);
  ret->begin = mem;
  ret->end = mem;
  ret->capacity = capacity;
  return (Array)ret;
}

array_t iarr_build(index_t element_size) {
  return (array_t) {
    .begin = NULL,
    .end = NULL,
    .element_size = element_size,
    .capacity = 0,
    .size = 0,
    .size_bytes = 0,
  };
}

array_t iarr_build_reserve(index_t element_size, index_t capacity) {
  void* data = NULL;
  if (capacity > 0) {
    data = malloc(element_size * capacity);
    assert(data);
  }

  return (array_t) {
    .begin = data,
    .end = data,
    .element_size = element_size,
    .capacity = MAX(capacity, 0),
    .size = 0,
    .size_bytes = 0,
  };
}

Array arr_copy(Array a_in) {
  DARRAY_INTERNAL;
  return arr_copy_span(a->span, a->element_size);
}

Array arr_copy_span(span_t span, index_t element_size) {
  assert(span.begin <= span.end);
  if (span.begin >= span.end) return iarr_new(element_size);
  index_t element_count = ispan_size(span, element_size);
  Array_Internal* ret = (Array_Internal*)iarr_new_reserve(
    element_size, element_count
  );
  index_t size_bytes = ispan_size_bytes(span);
  memcpy(ret->begin, span.begin, size_bytes);
  ret->size = size_bytes;
  return (Array)ret;
}

void arr_reserve(Array a_in, index_t capacity) {
  DARRAY_INTERNAL;
  if (a->size >= capacity) return;
  byte* new_data = realloc(a->begin, a->element_size * capacity);
  assert(new_data);
  a->begin = new_data;
  a->end = a->begin + a->element_size * a->capacity;
  a->capacity = capacity;
}

void arr_truncate(Array a_in, index_t max_size) {
  DARRAY_INTERNAL;
  if (a->capacity <= max_size) return;
  if (max_size <= 0) {
    arr_free(a_in);
    return;
  }
  byte* new_data = realloc(a->begin, a->element_size * max_size);
  assert(new_data);
  a->begin = new_data;
  a->capacity = max_size;
  if (a->size > max_size) {
    a->size = max_size;
    a->size_bytes = max_size * a->element_size;
  }
  a->end = a->begin + a->size * a->element_size;
}

void arr_trim(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size <= 0 || a->size == a->capacity) return;
  byte* new_data = realloc(a->begin, a->size * a->element_size);
  assert(new_data);
  a->begin = new_data;
  a->end = new_data + a->size * a->element_size;
}

void arr_clear(Array a_in) {
  DARRAY_INTERNAL;
  a->size = 0;
  a->size_bytes = 0;
  a->end = a->begin;
}

void arr_free(Array a_in) {
  if (!a_in) return;
  DARRAY_INTERNAL;
  if (!a->begin) return;
  arr_clear(a_in);
  free(a->begin);
  a->begin = NULL;
  a->end = NULL;
  a->capacity = 0;
}

void arr_delete(Array* a_in) {
  if (!a_in || !*a_in) return;
  Array_Internal* a = (Array_Internal*)*a_in;
  free(a->begin);
  free(a);
  *a_in = NULL;
}

span_t arr_release(Array* a_in) {
  if (!a_in || !*a_in) return span_empty;
  Array_Internal* a = (Array_Internal*)*a_in;
  span_t ret = a->span;
  free(a);
  *a_in = NULL;
  return ret;
}

index_t arr_write(Array a_in, index_t position, const void* element) {
  DARRAY_INTERNAL;
  assert(element);
  void* data = arr_emplace(a_in, position);
  if (!data) return 0;
  memcpy(data, element, a->element_size);
  return a->size;
}

index_t arr_write_back(Array a_in, const void* element) {
  DARRAY_INTERNAL;
  assert(element);
  void* data = arr_emplace_back(a_in);
  if (!data) return 0;
  memcpy(data, element, a->element_size);
  return a->size;
}

void* arr_emplace(Array a_in, index_t position) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  if (position >= a->size) {
    return arr_emplace_back(a_in);
  }
  if (a->size >= a->capacity) {
    arr_reserve(a_in, GROWTH_FACTOR);
  }
  byte* pos = a->data + a->element_size * position;
  memmove(pos + a->element_size, pos, a->size_bytes - position * a->element_size);
  a->size_bytes += a->element_size;
  a->end = a->begin + a->size_bytes;
  ++a->size;
  return pos;
}

void* arr_emplace_back(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size >= a->capacity) {
    arr_reserve(a_in, GROWTH_FACTOR);
  }
  a->size_bytes += a->element_size;
  a->end = a->begin + a->size_bytes;
  return a->data + a->size++ * a->element_size;
}

span_t arr_emplace_range(Array a_in, index_t position, index_t count) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  assert(count >= 0);
  if (position >= a->size) {
    return arr_emplace_back_range(a_in, count);
  }
  if (a->size + count >= a->capacity) {
    arr_reserve(a_in, a->size + count);
  }
  byte* pos = a->data + a->element_size * position;
  ptrdiff_t new_bytes = a->element_size * count;
  memmove(pos + new_bytes, pos, a->size_bytes - position * a->element_size);
  a->size_bytes += new_bytes;
  a->end = a->begin + a->size_bytes;
  a->size += count;
  return (span_t) {
    .begin = pos,
    .end = pos + new_bytes,
  };
}

span_t arr_emplace_back_range(Array a_in, index_t count) {
  DARRAY_INTERNAL;
  assert(count >= 0);
  if (a->size + count >= a->capacity) {
    arr_reserve(a_in, a->size + count);
  }
  void* ret = a->data + a->size * a->element_size;
  a->size_bytes += a->element_size * count;
  a->end = a->begin + a->size_bytes;
  a->size += count;
  return (span_t) {
    .begin = ret,
    .end = a->end,
  };
}

index_t arr_remove(Array a_in, index_t position) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  if (position >= a->size) return a->size;
  if (position == a->size - 1) return arr_pop_back(a_in);
  byte* pos = a->data + a->element_size * position;
  a->size_bytes -= a->element_size;
  a->end = a->begin + a->size_bytes;
  memmove(pos, pos + a->element_size, a->size_bytes);
  return --a->size;
}

index_t arr_remove_range(Array a_in, index_t position, index_t count) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  assert(count > 0);
  if (position >= a->size) return a->size;
  if (position + count >= a->size) {
    a->size = position;
    a->size_bytes = position * a->element_size;
    return position;
  }
  byte* pos = a->data + position * a->element_size;
  ptrdiff_t count_bytes = count * a->element_size;
  ptrdiff_t remainder_size = (a->size - position + count) * a->element_size;
  memmove(pos, pos + count_bytes, remainder_size);
  a->size -= count;
  a->size_bytes = a->size * a->element_size;
  a->end = a->begin + a->size_bytes;
  return a->size;
}

index_t arr_remove_unstable(Array a_in, index_t position) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  byte* last = arr_ref_back(a_in);
  if (last == NULL) return 0;
  if (position >= a->size) return a->size;
  if (position == a->size - 1) return arr_pop_back(a_in);
  byte* pos = a->data + a->element_size * position;
  memcpy(pos, last, a->element_size);
  a->size_bytes -= a->element_size;
  a->end = a->begin + a->size_bytes;
  return --a->size;
}

index_t arr_pop_back(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size <= 0) return 0;
  a->size_bytes -= a->element_size;
  a->end = a->begin + a->size_bytes;
  return --a->size;
}

void* arr_ref(Array a_in, index_t index) {
  DARRAY_INTERNAL;
  if (index < 0 || index >= a->size) return NULL;
  return a->data + index * a->element_size;
}

void* arr_ref_front(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size <= 0) return NULL;
  return a->data;
}

void* arr_ref_back(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size <= 0) return NULL;
  return a->data + (a->size - 1) * a->element_size;
}

bool arr_read(const Array a_in, index_t index, void* element) {
  DARRAY_INTERNAL_CONST;
  assert(element);
  assert(index >= 0);
  if (index >= a->size) return false;
  memcpy(element, a->data + index * a->element_size, a->element_size);
  return true;
}

bool arr_read_front(const Array a_in, void* element) {
  DARRAY_INTERNAL_CONST;
  assert(element);
  if (a->size <= 0) return false;
  memcpy(element, a->data, a->element_size);
  return true;
}

bool arr_read_back(const Array a_in, void* element) {
  DARRAY_INTERNAL_CONST;
  assert(element);
  if (a->size <= 0) return false;
  memcpy(element, a->data + (a->size - 1) * a->element_size, a->element_size);
  return true;
}

bool arr_contains(const Array a_in, const void* element) {
  DARRAY_INTERNAL_CONST;
  assert(element);
  if (a->size <= 0) return false;
  for (index_t i = 0; i < a->size; ++i) {
    if (!memcmp(element, a->data + i * a->element_size, a->element_size)) {
      return true;
    }
  }
  return false;
}

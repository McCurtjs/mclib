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

#include "array.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "types.h"

// internal opaque structure:
typedef struct Array_Internal {
  // public (read only)
  index_s element_size;
  index_s capacity;
  index_s size;
  index_s size_bytes;

  // private
  byte* data;
} Array_Internal;

#define DARRAY_STARTING_SIZE 2

#define GROWTH_FACTOR \
  MAX(DARRAY_STARTING_SIZE, a->capacity + a->capacity / 2)

#define DARRAY_INTERNAL \
  assert(a_in); \
  Array_Internal* a = (Array_Internal*)(a_in)

#define DARRAY_INTERNAL_CONST \
  assert(a_in); \
  const Array_Internal* a = (const Array_Internal*)(a_in)

Array _array_new_(index_s element_size) {
  Array_Internal* ret = malloc(sizeof(Array_Internal));
  assert(ret);
  *ret = (Array_Internal) {
    .element_size = element_size,
    .capacity = 0,
    .size = 0,
    .size_bytes = 0,
    .data = NULL,
  };
  return (Array)ret;
}

Array _array_new_reserve_(index_s element_size, index_s capacity) {
  Array_Internal* ret = malloc(sizeof(Array_Internal));
  assert(ret);
  *ret = (Array_Internal) {
    .element_size = element_size,
    .capacity = capacity,
    .size = 0,
    .size_bytes = 0,
    .data = malloc(element_size * capacity),
  };
  return (Array)ret;
}

void array_reserve(Array a_in, index_s capacity) {
  DARRAY_INTERNAL;
  if (!a || a->size >= capacity) return;
  void* new_data = realloc(a->data, a->element_size * capacity);
  assert(new_data); // TODO: better handling of critical memory situations
  a->data = new_data;
  a->capacity = capacity;
}

void array_truncate(Array a_in, index_s max_size) {
  DARRAY_INTERNAL;
  if (!a || a->capacity < max_size) return;
  void* new_data = realloc(a->data, a->element_size * max_size);
  if (!new_data) return;
  a->data = new_data;
  a->capacity = max_size;
  if (a->size > max_size) {
    a->size = max_size;
    a->size_bytes = max_size * a->element_size;
  }
}

void array_clear(Array a_in) {
  DARRAY_INTERNAL;
  a->size = 0;
  a->size_bytes = 0;
}

void array_free(Array a_in) {
  if (!a_in) return;
  DARRAY_INTERNAL;
  if (!a->data) return;
  array_clear(a_in);
  free(a->data);
  a->capacity = 0;
  a->data = NULL;
}

void array_delete(Array* a_in) {
  if (!a_in || !*a_in) return;
  Array_Internal* a = (Array_Internal*)*a_in;
  free(a->data);
  free(a);
  *a_in = NULL;
}

void* array_release(Array* a_in) {
  if (!a_in || !*a_in) return NULL;
  Array_Internal* a = (Array_Internal*)*a_in;
  void* ret = a->data;
  free(a);
  *a_in = NULL;
  return ret;
}

index_s array_write(Array a_in, index_s position, const void* element) {
  DARRAY_INTERNAL;
  assert(element);
  void* data = array_emplace(a_in, position);
  if (!data) return 0;
  memcpy(data, element, a->element_size);
  return a->size;
}

index_s array_write_back(Array a_in, const void* element) {
  DARRAY_INTERNAL;
  assert(element);
  void* data = array_emplace_back(a_in);
  if (!data) return 0;
  memcpy(data, element, a->element_size);
  return a->size;
}

void* array_emplace(Array a_in, index_s position) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  if (position >= a->size) {
    return array_emplace_back(a_in);
  }
  if (a->size >= a->capacity) {
    array_reserve(a_in, GROWTH_FACTOR);
  }
  byte* pos = a->data + a->element_size * position;
  memmove(pos + a->element_size, pos, a->size_bytes - position * a->element_size);
  a->size_bytes += a->element_size;
  ++a->size;
  return pos;
}

void* array_emplace_back(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size >= a->capacity) {
    array_reserve(a_in, GROWTH_FACTOR);
  }
  a->size_bytes += a->element_size;
  return a->data + a->size++ * a->element_size;
}

void* array_emplace_range(Array a_in, index_s position, index_s count) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  assert(count >= 0);
  if (position >= a->size) {
    return array_emplace_back_range(a_in, count);
  }
  if (a->size + count >= a->capacity) {
    array_reserve(a_in, a->size + count);
  }
  byte* pos = a->data + a->element_size * position;
  ptrdiff_t new_bytes = a->element_size * count;
  memmove(pos + new_bytes, pos, a->size_bytes - position * a->element_size);
  a->size_bytes += new_bytes;
  a->size += count;
  return pos;
}

void* array_emplace_back_range(Array a_in, index_s count) {
  DARRAY_INTERNAL;
  assert(count >= 0);
  if (a->size + count >= a->capacity) {
    array_reserve(a_in, a->size + count);
  }
  void* ret = a->data + a->size * a->element_size;
  a->size_bytes += a->element_size * count;
  a->size += count;
  return ret;
}

index_s array_remove(Array a_in, index_s position) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  if (position >= a->size) return a->size;
  if (position == a->size - 1) return array_pop_back(a_in);
  byte* pos = a->data + a->element_size * position;
  a->size_bytes -= a->element_size;
  memmove(pos, pos + a->element_size, a->size_bytes);
  return --a->size;
}

index_s array_remove_range(Array a_in, index_s position, index_s count) {
  DARRAY_INTERNAL;
  assert(position >= position);
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
  a->size_bytes = a->size * a->size_bytes;
  return a->size;
}

index_s array_remove_unstable(Array a_in, index_s position) {
  DARRAY_INTERNAL;
  assert(position >= 0);
  byte* last = array_ref_back(a_in);
  if (last == NULL) return 0;
  if (position >= a->size) return a->size;
  if (position == a->size - 1) return array_pop_back(a_in);
  byte* pos = a->data + a->element_size * position;
  memcpy(pos, last, a->element_size);
  a->size_bytes -= a->element_size;
  return --a->size;
}

index_s array_pop_back(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size <= 0) return 0;
  a->size_bytes -= a->element_size;
  return --a->size;
}

void* array_ref(Array a_in, index_s index) {
  DARRAY_INTERNAL;
  if (index >= a->size) return NULL;
  return a->data + index * a->element_size;
}

void* array_ref_front(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size <= 0) return NULL;
  return a->data;
}

void* array_ref_back(Array a_in) {
  DARRAY_INTERNAL;
  if (a->size <= 0) return NULL;
  return a->data + (a->size - 1) * a->element_size;
}

bool array_read(const Array a_in, index_s index, void* element) {
  DARRAY_INTERNAL_CONST;
  assert(element);
  assert(index >= 0);
  if (index >= a->size) return false;
  memcpy(element, a->data + index * a->element_size, a->element_size);
  return true;
}

bool array_read_front(const Array a_in, void* element) {
  DARRAY_INTERNAL_CONST;
  assert(element);
  if (a->size <= 0) return false;
  memcpy(element, a->data, a->element_size);
  return true;
}

bool array_read_back(const Array a_in, void* element) {
  DARRAY_INTERNAL_CONST;
  assert(element);
  if (a->size <= 0) return false;
  memcpy(element, a->data + (a->size - 1) * a->element_size, a->element_size);
  return true;
}

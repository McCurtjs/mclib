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

#include "packedmap.h"
#include "view.h"

#include <stdlib.h>
#include <memory.h>

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
#endif

typedef struct entry_t {
  index_t unique;
  union {
    index_t index;
    index_t free_list;
  };
  index_t reverse;
} entry_t;

typedef struct PackedMap_Internal {
  union {
    struct {
      byte* begin;
      byte* end;
    };
  };

  index_t element_size;
  index_t capacity;
  index_t size;
  index_t size_bytes;

  // Secrets
  index_t free_list;
  index_t unique_counter;
  entry_t* mapping;
} PackedMap_Internal;

#define PACKEDMAP_STARTING_SIZE 8

#define GROWTH_FACTOR \
  MAX(PACKEDMAP_STARTING_SIZE, pm->capacity + pm->capacity / 2)

#define PACKEDMAP_INTERNAL \
  PackedMap_Internal* pm = (PackedMap_Internal*)(pm_in); \
  assert(pm)

static inline byte* _get_slot(PackedMap_Internal* pm, index_t index) {
  return pm->begin + pm->element_size * index;
}

static void _reset_mapping_from(PackedMap_Internal* pm, index_t from) {
  for (index_t i = from; i < pm->capacity; ++i) {
    pm->mapping[i] = (entry_t) {
      .unique = 0,
      .index = 0,
      .reverse = 0,
    };
  }
}

PackedMap ipmap_new(index_t element_size) {
  PackedMap_Internal* ret = malloc(sizeof(PackedMap_Internal));
  assert(ret);
  *ret = (PackedMap_Internal) {
    .begin = NULL,
    .end = NULL,
    .element_size = element_size,
    .capacity = 0,
    .size = 0,
    .free_list = -1,
    .unique_counter = 0,
    .mapping = NULL,
  };
  return (PackedMap)ret;
}

PackedMap ipmap_new_reserve(index_t element_size, index_t capacity) {
  PackedMap_Internal* ret = (PackedMap_Internal*)ipmap_new(element_size);
  if (capacity <= 0) return (PackedMap)ret;
  byte* mem = malloc(ret->element_size * capacity);
  entry_t* map = malloc(sizeof(entry_t) * capacity);
  assert(mem);
  assert(map);
  ret->begin = mem;
  ret->end = mem;
  ret->capacity = capacity;
  ret->mapping = map;
  _reset_mapping_from(ret, 0);
  return (PackedMap)ret;
}

void pmap_reserve(PackedMap pm_in, index_t capacity) {
  PACKEDMAP_INTERNAL;
  if (pm->size >= capacity) return;
  byte* new_data = realloc(pm->begin, pm->element_size * capacity);
  entry_t* new_mapping = realloc(pm->mapping, sizeof(entry_t) * capacity);
  assert(new_data);
  assert(new_mapping);
  index_t old_capacity = pm->capacity;
  pm->begin = new_data;
  pm->end = _get_slot(pm, pm->size),
  pm->capacity = capacity;
  pm->mapping = new_mapping;
  _reset_mapping_from(pm, old_capacity);
}

void pmap_trim(PackedMap pm_in) {
  UNUSED(pm_in);
  assert(false && "TODO");
}

void pmap_clear(PackedMap pm_in) {
  PACKEDMAP_INTERNAL;
  pm->end = pm->begin;
  pm->size = 0;
  pm->size_bytes = 0;
  _reset_mapping_from(pm, 0);
}

void pmap_free(PackedMap pm_in) {
  if (!pm_in) return;
  PACKEDMAP_INTERNAL;
  if (!pm->begin && !pm->mapping) return;
  pmap_clear(pm_in);
  free(pm->begin);
  free(pm->mapping);
  pm->begin = NULL;
  pm->end = NULL;
  pm->mapping = NULL;
  pm->capacity = 0;
}

void pmap_delete(PackedMap* pm_in) {
  if (!pm_in || !*pm_in) return;
  PackedMap_Internal* pm = (PackedMap_Internal*)*pm_in;
  free(pm->begin);
  free(pm->mapping);
  free(pm);
  *pm_in = NULL;
}

void* pmap_emplace(PackedMap pm_in, slotkey_t* out_key) {
  PACKEDMAP_INTERNAL;
  assert(out_key);
  entry_t* mapping;
  index_t map_index;
  index_t slot_index = pm->size;
  if (pm->free_list >= 0) {
    map_index = pm->free_list;
    mapping = &pm->mapping[map_index];
    assert(mapping->unique == 0);
    pm->free_list = mapping->free_list;
  }
  else {
    map_index = pm->size;
    if (map_index >= pm->capacity) {
      pmap_reserve(pm_in, GROWTH_FACTOR);
    }
    mapping = &pm->mapping[map_index];
  }
  mapping->unique = ++pm->unique_counter;
  assert(mapping->unique);
  mapping->index = pm->size;
  pm->mapping[slot_index].reverse = map_index;
  *out_key = (slotkey_t) {
    .index = slot_index,
    .unique = mapping->unique,
  };
  pm->size_bytes += pm->element_size;
  pm->end += pm->element_size;
  ++pm->size;
  return _get_slot(pm, slot_index);
}

slotkey_t pmap_insert(PackedMap pm_in, const void* element) {
  assert(element);
  slotkey_t ret;
  void* data = pmap_emplace(pm_in, &ret);
  memcpy(data, element, pm_in->element_size);
  return ret;
}

slotkey_t pmap_key(PackedMap pm_in, index_t index) {
  PACKEDMAP_INTERNAL;
  if (index < 0 || index >= pm->size) return (slotkey_t) { 0 };
  entry_t entry = pm->mapping[index];
  index_t map_index = entry.reverse;
  entry = pm->mapping[map_index];
  return (slotkey_t) {
    .index = map_index,
    .unique = entry.unique
  };
}

void* pmap_ref(PackedMap pm_in, slotkey_t key) {
  PACKEDMAP_INTERNAL;
  if (key.index < 0 || key.index >= pm->capacity) return NULL;
  entry_t mapping = pm->mapping[key.index];
  if (mapping.unique != key.unique) return NULL;
  return _get_slot(pm, mapping.index);
}

bool pmap_read(PackedMap pm_in, slotkey_t key, void* out_element) {
  assert(out_element);
  const void* ref = pmap_ref(pm_in, key);
  if (!ref) return false;
  memcpy(out_element, ref, pm_in->element_size);
  return true;
}

bool pmap_contains(PackedMap pm_in, slotkey_t key) {
  PACKEDMAP_INTERNAL;
  if (key.index < 0 || key.index >= pm->capacity) return false;
  return pm->mapping[key.index].unique == key.unique;
}

#include <ctype.h>

bool pmap_remove(PackedMap pm_in, slotkey_t key) {
  PACKEDMAP_INTERNAL;

  // Check index validity and validate unique identifier
  if (key.index < 0 || key.index >= pm->capacity) return false;
  entry_t* mapping = &pm->mapping[key.index];
  if (mapping->unique != key.unique) return false;

  // Move data from the last slot into the now empty one
  index_t slot_index = mapping->index;
  index_t last_slot_index = pm->size - 1;
  byte* data_slot = _get_slot(pm, slot_index);
  byte* data_last = _get_slot(pm, last_slot_index);
  if (data_last != data_slot) {
    memcpy(data_slot, data_last, pm->element_size);
  }

  // Update the last-item's new references to point to each other, drop old key
  index_t map_reverse_index = pm->mapping[last_slot_index].reverse;
  pm->mapping[slot_index].reverse = map_reverse_index;
  pm->mapping[map_reverse_index].index = slot_index;
  mapping->unique = 0;
  mapping->free_list = pm->free_list;

  // Invalidate the removed key and put it on the free list
  pm->free_list = key.index;
  pm->size_bytes -= pm->element_size;
  pm->end -= pm->element_size;
  --pm->size;
  return true;
}

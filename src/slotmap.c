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

#include "slotmap.h"
#include "array.h"

#include <stdlib.h>
#include <memory.h>

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
#endif

typedef struct SlotMap_Internal {
  index_t   element_size;
  index_t   capacity;
  index_t   size;
  index_t   size_bytes;

  // Secrets
  index_t   slot_size;
  int32_t   free_list;
  uint64_t  unique_counter;
  byte*     begin;
} SlotMap_Internal;

typedef struct slot_t {
  uint64_t unique;
  byte data[];
} slot_t;

#define STARTING_SIZE 8
#define EMPTY_FREELIST SK_INDEX_MAX
#define EMPTY_SLOT 0

#define GROWTH_FACTOR \
  MIN(SK_INDEX_MAX, MAX(STARTING_SIZE, sm->capacity + sm->capacity / 2))

#define SLOTMAP_INTERNAL \
  SlotMap_Internal* sm = (SlotMap_Internal*)(sm_in); \
  assert(sm)

static inline slot_t* _get_slot(SlotMap_Internal* sm, index_t index) {
  return (void*)(sm->begin + sm->slot_size * index);
}

static inline int32_t* _slot_free_list(slot_t* slot) {
  return (int32_t*)&slot->data[0];
}

static inline void _reset_slots_from(SlotMap_Internal* sm, index_t from) {
  for (index_t i = from; i < sm->capacity; ++i) {
    _get_slot(sm, i)->unique = EMPTY_SLOT;
  }
}

SlotMap ismap_new(index_t element_size) {
  SlotMap_Internal* ret = malloc(sizeof(SlotMap_Internal));
  assert(ret);
  // use sizeof(index_t) for hopefully better slot alignment?
  // the space needed is for the int32 free_list, so it should fit regardless.
  *ret = (SlotMap_Internal) {
    .element_size = element_size,
    .capacity = 0,
    .size = 0,
    .slot_size = sizeof(slot_t) + MAX(element_size, sizeof(index_t)),
    .free_list = EMPTY_FREELIST,
    .unique_counter = 0,
    .begin = NULL,
  };
  return (SlotMap)ret;
}

SlotMap ismap_new_reserve(index_t element_size, index_t capacity) {
  SlotMap_Internal* ret = (SlotMap_Internal*)ismap_new(element_size);
  if (capacity <= 0) return (SlotMap)ret;
  assert(capacity <= SK_INDEX_MAX);
  byte* mem = malloc(ret->slot_size * capacity);
  assert(mem);
  ret->begin = mem;
  ret->capacity = capacity;
  _reset_slots_from(ret, 0);
  return (SlotMap)ret;
}

void smap_reserve(SlotMap sm_in, index_t capacity) {
  SLOTMAP_INTERNAL;
  if (sm->size >= capacity) return;
  assert(capacity <= SK_INDEX_MAX);
  byte* new_data = realloc(sm->begin, sm->slot_size * capacity);
  assert(new_data);
  index_t old_capacity = sm->capacity;
  sm->begin = new_data;
  sm->capacity = capacity;
  _reset_slots_from(sm, old_capacity);
}

void smap_trim(SlotMap sm_in) {
  UNUSED(sm_in);
  assert(false && "TODO");
}

void smap_clear(SlotMap sm_in) {
  SLOTMAP_INTERNAL;
  sm->size = 0;
  _reset_slots_from(sm, 0);
}

void smap_free(SlotMap sm_in) {
  if (!sm_in) return;
  SLOTMAP_INTERNAL;
  if (!sm->begin) return;
  smap_clear(sm_in);
  free(sm->begin);
  sm->begin = NULL;
  sm->capacity = 0;
}

void smap_delete(SlotMap* sm_in) {
  if (!sm_in || !*sm_in) return;
  SlotMap_Internal* sm = (SlotMap_Internal*)*sm_in;
  free(sm->begin);
  free(sm);
  *sm_in = NULL;
}

void* smap_emplace(SlotMap sm_in, slotkey_t* out_key) {
  SLOTMAP_INTERNAL;
  assert(out_key);
  slot_t* slot;
  int32_t index;
  if (sm->free_list != EMPTY_FREELIST) {
    index = sm->free_list;
    slot = _get_slot(sm, index);
    assert(slot->unique == 0);
    sm->free_list = *_slot_free_list(slot);
  }
  else {
    index = (int32_t)sm->size;
    if (index >= sm->capacity) {
      smap_reserve(sm_in, GROWTH_FACTOR);
    }
    slot = _get_slot(sm, index);
  }
  slot->unique = ++sm->unique_counter;
  assert(slot->unique && slot->unique <= SK_UNIQUE_MAX);
  *out_key = sk_build(index, slot->unique);
  ++sm->size;
  return slot->data;
}

slotkey_t smap_insert(SlotMap sm_in, const void* element) {
  assert(element);
  slotkey_t ret;
  void* data = smap_emplace(sm_in, &ret);
  memcpy(data, element, sm_in->element_size);
  return ret;
}

void* smap_ref(SlotMap sm_in, slotkey_t key) {
  SLOTMAP_INTERNAL;
  int32_t key_index = sk_index(key);
  if (key_index < 0 || key_index >= sm->capacity) return NULL;
  slot_t* slot = _get_slot(sm, key_index);
  if (slot->unique != sk_unique(key)) return NULL;
  return slot->data;
}

bool smap_read(SlotMap sm, slotkey_t key, void* out_element) {
  assert(out_element);
  const void* ref = smap_ref(sm, key);
  if (!ref) return false;
  memcpy(out_element, ref, sm->element_size);
  return true;
}

void* smap_next(SlotMap sm_in, slotkey_t* iterator) {
  SLOTMAP_INTERNAL;
  assert(iterator);
  int32_t index = sk_index(*iterator) + 1;
  assert(index >= 0 && index < SK_INDEX_MAX);
  if (iterator->hash == SK_NULL.hash) index = 0;
  for (; index < sm->capacity; ++index) {
    slot_t* slot = _get_slot(sm, index);
    if (slot->unique != EMPTY_SLOT) {
      *iterator = sk_build(index, slot->unique);
      return slot->data;
    }
  }
  *iterator = SK_NULL;
  return NULL;
}

bool smap_contains(SlotMap sm_in, slotkey_t key) {
  SLOTMAP_INTERNAL;
  int32_t key_index = sk_index(key);
  if (key_index < 0 || key_index >= sm->capacity) return false;
  return _get_slot(sm, key_index)->unique == sk_unique(key);
}

bool smap_remove(SlotMap sm_in, slotkey_t key) {
  SLOTMAP_INTERNAL;
  int32_t key_index = sk_index(key);
  if (key_index < 0 || key_index >= sm->capacity) return false;
  slot_t* slot = _get_slot(sm, key_index);
  if (slot->unique != sk_unique(key)) return false;
  slot->unique = EMPTY_SLOT;
  *_slot_free_list(slot) = sm->free_list;
  sm->free_list = key_index;
  --sm->size;
  return true;
}

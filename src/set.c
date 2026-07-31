/*******************************************************************************
* MIT License
*
* Copyright (c) 2026 Curtis McCoy
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

#define MCLIB_INTERNAL_IMPL
#include "set.h"

#include "utility.h" // msb, hash

#include <stdlib.h> // malloc/free
#include <string.h> // memcmp, memcpy
#include <stddef.h> // offsetof

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
# pragma warning ( disable : 28182 )
#endif

#define MIN_CAPACITY 8

typedef struct Set_Cell {
  // Contextual 'next' pointer, depending on if the cell is in use
  // The 'free' list is a double-linked list
  // The 'bucket' list is a single-linked ring-list
  union {
    struct Set_Cell* free_next;
    struct Set_Cell* bucket_next;
  };

  // Hash for the item in this cell, or 0 if it isn't occupied
  hash_t hash;

  // Contextual 'prev' pointer for the free list, or location of the key if used
  union {
    struct Set_Cell* free_prev;
    byte element_start;
  };

  // value
} Set_Cell;

// internal opaque structure:
typedef struct Set_Internal {
  struct _opaque_Set_base_t pub;

  // private
  index_t     cell_size;

  compare_fn  element_compare;
  hash_fn     element_hash;
  copy_fn     element_copy;
  delete_fn   element_delete;

  Set_Cell*   free_list;

  byte*       data;
} Set_Internal;

#define HSET_INTERNAL \
  Set_Internal* s = (Set_Internal*)(s_in); \
  assert(s)

////////////////////////////////////////////////////////////////////////////////
// Static helpers for key and cell manipulation
////////////////////////////////////////////////////////////////////////////////

// Helper to resolve hash function - use murmur3 by default if none is provided.
static hash_t _value_hash(Set_Internal* s, const void* value) {
  hash_t ret = s->element_hash(value, s->pub.element_size);

  // A hash value of 0x00 is used to denote an empty cell, so force to 1 if we
  //    somehow actually hash to that.
  return ret ? ret : 1;
}

// Helper to get value from cell. When occupied, it follows the key position.
static void* _cell_value(Set_Cell* cell) {
  return &cell->element_start;
}

// Helper to search a slot bucket for a given value's cell, if present.
static Set_Cell* _cell_search_bucket(
  Set_Internal* s, Set_Cell* bucket, const void* value, hash_t hash
) {
  Set_Cell* cell = bucket;

  do {

    size_t element_size = (size_t)s->pub.element_size;

    if (cell->hash == hash
    && !s->element_compare(value, _cell_value(cell), element_size)
    ) {
      return cell;
    }

    cell = cell->bucket_next;
  } while (cell != bucket);

  return NULL;
}

static Set_Cell* _cell_bucket_end(Set_Cell* bucket) {
  Set_Cell* cell = bucket;
  while (cell->bucket_next != bucket) cell = cell->bucket_next;
  return cell;
}

// Helper to move a cell's contents from one slot to another and update bucket
static void _cell_move(Set_Internal* s, Set_Cell* dst, Set_Cell* src) {
  dst->hash = src->hash;
  memcpy(_cell_value(dst), _cell_value(src), s->pub.element_size);

  Set_Cell* cell = _cell_bucket_end(src);
  cell->bucket_next = dst;
  dst->bucket_next = src->bucket_next;
}

// Helper to remove a node from the free list.
static void _cell_remove_from_free_list(Set_Internal* s, Set_Cell* cell) {
  if (cell->free_next) cell->free_next->free_prev = cell->free_prev;
  if (cell->free_prev) cell->free_prev->free_next = cell->free_next;
  else s->free_list = cell->free_next;
}

static void _cell_move_to_free_list(Set_Internal* s, Set_Cell* cell) {
  cell->hash = 0;
  cell->free_prev = NULL;
  cell->free_next = s->free_list;
  s->free_list->free_prev = cell;
  s->free_list = cell;
}

static Set_Cell* _cell_take_from_free_list(Set_Internal* s) {
  assert(s->pub.size < s->pub.capacity);
  assert(s->free_list);
  Set_Cell* ret = s->free_list;
  s->free_list = ret->free_next;
  s->free_list->free_prev = NULL;
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Static set helpers
////////////////////////////////////////////////////////////////////////////////

typedef union _ensure_t {
  set_ensure_t result;
  struct {
    void* value;
    bool is_new;
  };
} _ensure_t;

static _ensure_t _set_ensure(Set_Internal*, const void* value, hash_t hash);

// Helper to clear out the contents of the set and reset the free list.
static void _set_clear(Set_Internal* m) {
  m->free_list = (Set_Cell*)m->data;

  if (!m->free_list) return;

  // Link the map slots together to create the free list.
  Set_Cell* cell = m->free_list;
  cell->free_prev = NULL;

  for (index_t i = 0;;) {
    cell->free_next = (Set_Cell*)((byte*)cell + m->cell_size);
    cell->hash = 0;

    until(++i >= m->pub.capacity);

    Set_Cell* prev = cell;
    cell = cell->free_next;
    cell->free_prev = prev;
  }

  cell->free_next = NULL;
}

// Helper to initialize an empty map with valid free list.
static void _set_initialize(Set_Internal* m, index_t new_size) {
  m->pub.capacity = MAX(msb(new_size) << 1, MIN_CAPACITY);
  m->pub.size = 0;
  m->data = malloc(m->cell_size * m->pub.capacity);
  assert(m->data);
  _set_clear(m);
}

// Helper to get the expected slot for the given hash value.
static Set_Cell* _set_get_slot(Set_Internal* s, hash_t hash) {
  assert(hash); // can't be 0
  if (s->pub.capacity <= 0) return NULL;
  hash_t index = hash & (s->pub.capacity - 1); // 0x010000 -> 0x001111
  Set_Cell* cell = (Set_Cell*)(s->data + s->cell_size * index);
  return cell;
}

// Helper to get the expected slot, but will also initialize the array if empty.
static Set_Cell* _set_get_slot_init(Set_Internal* s, hash_t hash) {
  if (s->pub.capacity == 0) _set_initialize(s, 1);
  return _set_get_slot(s, hash);
}

// Helper to copy the contents of an old map into a new map.
static void _set_move(Set_Internal* s, void* old_data, index_t old_capacity) {
  Set_Cell* cell = old_data;

  for (index_t i = 0; i < old_capacity; ++i) {
    if (cell->hash != 0) {
      void* value = _cell_value(cell);
      _ensure_t slot = _set_ensure(s, value, cell->hash);
      assert(slot.is_new);

      memcpy(slot.value, _cell_value(cell), s->pub.element_size);
    }

    cell = (Set_Cell*)((byte*)cell + s->cell_size);
  }
}

// Helper to check if the map needs to be expanded, and copy values over if so.
static bool _set_check_expand(Set_Internal* s, index_t new_size) {
  index_t cap75 = (s->pub.capacity >> 2) + (s->pub.capacity >> 1);

  // Don't need to expand if the new size is under 75% of capacity
  if (new_size < cap75) {
    return false;
  }

  // Handle the case for our first allocation
  // TODO: is this redundant after adding _map_get_slot_init?
  //if (!s->data) {
  //  _set_initialize(s, new_size);
  //  return true;
  //}

  // TODO: If we're set to not expand, don't
  if (s->pub.fixed_size) return false;

  byte* old_data = s->data;
  index_t old_capacity = s->pub.capacity;
  _set_initialize(s, MAX(new_size, s->pub.capacity));
  _set_move(s, old_data, old_capacity);
  free(old_data);
  return true;
}

// Helper to delete the values if a delete function is given.
void _set_delete_contents(Set_Internal* s) {
  if (s->element_delete) {
    byte* bcell = s->data;
    for (index_t i = 0; i < s->pub.capacity; ++i, bcell += s->cell_size) {
      Set_Cell* cell = (Set_Cell*)bcell;
      if (cell->hash) s->element_delete(_cell_value(cell));
    }
  }
}

// Helper to perform the 'ensure' operation to insert a value to the map.
// The returned cell from this function will not have its key updated if new.
static _ensure_t _set_ensure(Set_Internal* s, const void* value, hash_t hash) {
  assert(value);
  assert(hash == _value_hash(s, value)); // redundant, ensure correct in test

  Set_Cell* slot = _set_get_slot_init(s, hash);

  // if the slot is occupied, check if our value is already in the map
  if (slot->hash) {
    Set_Cell* cell = _cell_search_bucket(s, slot, value, hash);

    if (cell) {
      return (_ensure_t) {
        .value = _cell_value(cell),
        .is_new = false,
      };
    }
  }

  // if the set capacity is locked, check if we've reached capacity
  if (s->pub.fixed_size && s->pub.size >= s->pub.capacity) {
    return (_ensure_t) { .value = NULL, .is_new = false };
  }

  // if our key is not already in the set, check expansion and update slot
  if (_set_check_expand(s, s->pub.size + 1)) {
    slot = _set_get_slot(s, hash);
  }

  // if the slot is occupied, check if the occupant actually belongs there
  if (slot->hash) {
    Set_Cell* cell = _set_get_slot(s, slot->hash);

    if (slot != cell) {
      _cell_move(s, _cell_take_from_free_list(s), slot);
      slot->bucket_next = slot;
    }

    // the other cell does belong here, so add the new key to its bucket
    else {
      cell = _cell_take_from_free_list(s);
      cell->bucket_next = slot->bucket_next;
      slot->bucket_next = cell;
      slot = cell;
    }
  }

  // if the slot was unoccupied, we only have to remove it from the free list
  else {
    _cell_remove_from_free_list(s, slot);
    slot->bucket_next = slot; // start of bucket ring list
  }

  // with the correct slot in hand, update the value and return
  ++s->pub.size;
  slot->hash = hash;

  return (_ensure_t) {
    .value = _cell_value(slot),
    .is_new = true,
  };
}

////////////////////////////////////////////////////////////////////////////////
// Set construction/destruction
////////////////////////////////////////////////////////////////////////////////

HSet iset_new(
  index_t element_size, hash_fn key_hash, compare_fn key_compare
) {
  Set_Internal* ret = malloc(sizeof(Set_Internal));
  assert(ret);

  // Ensure the key/value pair is large enough to sub for the free_prev pointer.
  index_t value_size = element_size;
  if ((size_t)value_size < sizeof(void*)) value_size = sizeof(void*);

  *ret = (Set_Internal) {
    .pub = {
      .size = 0,
      .capacity = 0,
      .element_size = element_size,
      .fixed_size = false,
    },
    .cell_size = sizeof(Set_Cell) + value_size - sizeof(void*),
    .element_compare = key_compare ? key_compare : memcmp,
    .element_hash = key_hash ? key_hash : hash,
    .element_copy = memcpy,
    .element_delete = NULL,
    .free_list = NULL,
    .data = NULL,
  };
  return (HSet)ret;
}

void set_callbacks_element(HSet s_in, copy_fn el_copy, delete_fn el_delete) {
  HSET_INTERNAL;
  assert(s->pub.size == 0);
  s->element_copy = el_copy ? el_copy : memcpy;
  s->element_delete = el_delete;
}

HSet set_copy(HSet s_in) {
  //HSET_INTERNAL;
  (void)s_in;
  return NULL;
}

void set_reserve(HSet s_in, index_t capacity) {
  HSET_INTERNAL;
  assert(capacity >= 0);
  if (capacity <= s->pub.capacity) return;

  if (!s->data) {
    _set_initialize(s, capacity);
  }
  else {
    void* old_data = s->data;
    index_t old_capacity = s->pub.capacity;
    _set_initialize(s, capacity);
    _set_move(s, old_data, old_capacity);
    free(old_data);
  }
}

void set_delete(HSet* s_in) {
  if (!s_in || !*s_in) return;
  Set_Internal* s = *(Set_Internal**)s_in;
  _set_delete_contents(s);
  free(s->data);
  free(s);
  *s_in = NULL;
}

void set_clear(HSet s_in) {
  HSET_INTERNAL;
  _set_delete_contents(s);
  _set_clear(s);
  s->pub.size = 0;
}

void set_free(HSet s_in) {
  HSET_INTERNAL;
  _set_delete_contents(s);
  free(s->data);
  s->pub.size = 0;
  s->pub.capacity = 0;
  s->free_list = NULL;
  s->data = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Inserting elements
////////////////////////////////////////////////////////////////////////////////

set_ensure_t set_ensure(HSet s_in, const void* value) {
  HSET_INTERNAL;
  assert(value);
  _ensure_t ret = _set_ensure(s, value, _value_hash(s, value));
  assert(ret.value);
  if (ret.is_new) s->element_copy(ret.value, value, s->pub.element_size);
  return ret.result;
}

bool set_write(HSet s_in, const void* value) {
  HSET_INTERNAL;
  assert(value);
  _ensure_t result = _set_ensure(s, value, _value_hash(s, value));
  assert(result.value);
  if (result.value != value) {
    if (!result.is_new) {
      // because the cell hash is keyed to the value, we can only overwrite the
      //    value if it's the same (ex: changing referenced memory for a string)
      if (s->element_compare(result.value, value, s->pub.element_size)) {
        return false;
      }
      if (s->element_delete) s->element_delete(result.value);
    }
    s->element_copy(result.value, value, s->pub.element_size);
  }
  return result.is_new;
}

bool set_insert(HSet s_in, const void* value) {
  HSET_INTERNAL;
  assert(value);
  _ensure_t result = _set_ensure(s, value, _value_hash(s, value));
  if (!result.is_new || !result.value) return false;
  s->element_copy(result.value, value, s->pub.element_size);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Indexing and access
////////////////////////////////////////////////////////////////////////////////

const void* set_ref(HSet s_in, const void* value) {
  HSET_INTERNAL;
  assert(value);
  hash_t hash = _value_hash(s, value);
  Set_Cell* slot = _set_get_slot(s, hash);
  if (!slot || !slot->hash) return NULL;
  Set_Cell* cell = _cell_search_bucket(s, slot, value, hash);
  if (!cell) return NULL;
  return _cell_value(cell);
}

bool set_contains(HSet s_in, const void* value) {
  return set_ref(s_in, value) != NULL;
}

const void* set_next(HSet s_in, const void* iterator) {
  HSET_INTERNAL;
  if (!s->pub.size) return NULL;

  const byte* const data_end = s->data + s->pub.capacity * s->cell_size;

  // get address of next slot in the set
  const byte* bval = iterator;

  if (bval) {
    assert(bval >= s->data);
    bval += s->cell_size;
  } else {
    bval = _cell_value((Set_Cell*)s->data);
  }

  // get cell for that key
  Set_Cell* cell = (Set_Cell*)(bval - offsetof(Set_Cell, element_start));

  // find the next occupied cell
  while ((byte*)cell < data_end) {
    if (cell->hash) {
      return _cell_value(cell);
    }
    cell = (Set_Cell*)((byte*)cell + s->cell_size);
  }

  return NULL;
}

void set_process(HSet s_in, set_process_fn processor) {
  UNUSED(s_in);
  UNUSED(processor);
}

////////////////////////////////////////////////////////////////////////////////
// Removal
////////////////////////////////////////////////////////////////////////////////

bool set_remove_hash(HSet s_in, const void* value, hash_t hash) {
  HSET_INTERNAL;
  assert(value);
  assert(hash);

  Set_Cell* slot = _set_get_slot(s, hash);
  if (!slot || !slot->hash) return false;

  Set_Cell* cell = _cell_search_bucket(s, slot, value, hash);
  if (!cell) return false;

  if (s->element_delete) {
    s->element_delete(_cell_value(cell));
  }

  // if this is the last item in the bucket, free the slot
  if (cell == slot && slot->bucket_next == slot) {
    _cell_move_to_free_list(s, slot);
  }

  // if this in the main slot but there are others in the bucket, shift one here
  else if (cell == slot) {
    Set_Cell* next = slot->bucket_next;
    _cell_move(s, slot, next);
    _cell_move_to_free_list(s, next);
  }

  // if there are other items in the bucket, but this isn't the main slot
  else {
    Set_Cell* prev = _cell_bucket_end(cell);
    prev->bucket_next = cell->bucket_next;
    _cell_move_to_free_list(s, cell);
  }

  --s->pub.size;
  return true;
}

bool set_remove(HSet s_in, const void* value) {
  HSET_INTERNAL;
  assert(value);
  hash_t hash = _value_hash(s, value);
  return set_remove_hash(s_in, value, hash);
}

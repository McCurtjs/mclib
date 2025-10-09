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

#include "map.h"
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

typedef struct Map_Cell {
  // Contextual 'next' pointer, depending on if the cell is in use
  // The 'free' list is a double-linked list
  // The 'bucket' list is a single-linked ring-list
  union {
    struct Map_Cell* free_next;
    struct Map_Cell* bucket_next;
  };

  // Hash for the item in this cell, or 0 if it isn't occupied
  hash_t hash;

  // Contextual 'prev' pointer for the free list, or location of the key if used
  union {
    struct Map_Cell* free_prev;
    byte key_start;
  };

  // value
} Map_Cell;

// internal opaque structure:
typedef struct Map_Internal {
  // public (read only)
  index_t size;
  index_t capacity;
  index_t key_size;
  index_t element_size;

  // private
  index_t cell_size;

  compare_fn compare_keys;
  hash_fn hash_key;
  delete_fn delete_value;
  delete_fn delete_key;

  Map_Cell* free_list;

  byte* data;
  // Array_Internal size space to avoid extra jump?
} Map_Internal;

#define HMAP_INTERNAL \
  Map_Internal* m = (Map_Internal*)(m_in); \
  assert(m)

// Helper to resolve hash function - use murmur3 by default if none is provided.
static hash_t _key_hash(Map_Internal* m, const void* key) {
  hash_t ret;

  if (m->hash_key) {
    ret = m->hash_key(key);
  } else {
    ret = hash(key, m->key_size);
  }

  // A hash value of 0x00 is used to denote an empty cell, so force to 1 if we
  //    somehow actually hash to that.
  return ret ? ret : 1;
}

// Helper to resolve key comparison - use memcmp by default is none is provided.
static bool _key_compare(Map_Internal* m, const void* lhs, const void* rhs) {
  if (m->compare_keys) {
    return !m->compare_keys(lhs, rhs);
  }
  return !memcmp(lhs, rhs, m->key_size);
}

// Helper to get key pointer from cell
static void* _cell_key(Map_Cell* cell) {
  return &cell->key_start;
}

// Helper to get value from cell. When occupied, it follows the key position.
static void* _cell_value(Map_Internal* m, Map_Cell* cell) {
  return &cell->key_start + m->key_size;
}

// Helper to search a slot bucket for a given key's cell, if present.
static Map_Cell* _cell_search_bucket(
  Map_Internal* m, Map_Cell* bucket, const void* key, hash_t hash
) {
  Map_Cell* cell = bucket;

  do {

    if (cell->hash == hash && _key_compare(m, key, _cell_key(cell))) {
      return cell;
    }

    cell = cell->bucket_next;
  } while (cell != bucket);

  return NULL;
}

static Map_Cell* _cell_bucket_end(Map_Cell* bucket) {
  Map_Cell* cell = bucket;
  while (cell->bucket_next != bucket) cell = cell->bucket_next;
  return cell;
}

// Helper to move a cell's contents from one slot to another and update bucket
static void _cell_move(Map_Internal* m, Map_Cell* dst, Map_Cell* src) {
  dst->hash = src->hash;
  memcpy(_cell_key(dst), _cell_key(src), m->key_size);
  memcpy(_cell_value(m, dst), _cell_value(m, src), m->element_size);

  Map_Cell* cell = _cell_bucket_end(src);
  cell->bucket_next = dst;
  dst->bucket_next = src->bucket_next;
}

// Helper to remove a node from the free list.
static void _cell_remove_from_free_list(Map_Internal* m, Map_Cell* cell) {
  if (cell->free_next) cell->free_next->free_prev = cell->free_prev;
  if (cell->free_prev) cell->free_prev->free_next = cell->free_next;
  else m->free_list = cell->free_next;
}

static void _cell_move_to_free_list(Map_Internal* m, Map_Cell* cell) {
  cell->hash = 0;
  cell->free_prev = NULL;
  cell->free_next = m->free_list;
  m->free_list->free_prev = cell;
  m->free_list = cell;
}

static Map_Cell* _cell_take_from_free_list(Map_Internal* m) {
  assert(m->size < m->capacity);
  assert(m->free_list);
  Map_Cell* ret = m->free_list;
  m->free_list = ret->free_next;
  m->free_list->free_prev = NULL;
  return ret;
}

static void _map_clear(Map_Internal* m) {
  m->free_list = (Map_Cell*)m->data;

  // Link the map slots together to create the free list.
  Map_Cell* cell = m->free_list;
  cell->free_prev = NULL;

  for (index_t i = 0;;) {
    cell->free_next = (Map_Cell*)((byte*)cell + m->cell_size);
    cell->hash = 0;

    until(++i >= m->capacity);

    Map_Cell* prev = cell;
    cell = cell->free_next;
    cell->free_prev = prev;
  }

  cell->free_next = NULL;
}

// Helper to initialize an empty map with valid free list.
static void _map_initialize(Map_Internal* m, index_t new_size) {
  m->capacity = MAX(msb(new_size) << 1, MIN_CAPACITY);
  m->size = 0;
  m->data = malloc(m->cell_size * m->capacity);
  assert(m->data);
  _map_clear(m);
}

// Helper to get the expected slot for the given hash value.
static Map_Cell* _map_get_slot(Map_Internal* m, hash_t hash) {
  assert(hash); // can't be 0
  if (m->capacity <= 0) return NULL;
  hash_t index = hash & (m->capacity - 1); // 0x010000 -> 0x001111
  Map_Cell* cell = (Map_Cell*)(m->data + m->cell_size * index);
  return cell;
}

// Helper to get the expected slot, but will also initialize the array if empty.
static Map_Cell* _map_get_slot_init(Map_Internal* m, hash_t hash) {
  if (m->capacity == 0) _map_initialize(m, 1);
  return _map_get_slot(m, hash);
}

// predeclare because this was removed from the header
bool map_write_hash(HMap m_in, const void* key, const void* val, hash_t hash);

// Helper to copy the contents of an old map into a new map.
static void _map_clone(Map_Internal* m, void* old_data, index_t old_capacity) {
  Map_Cell* cell = old_data;

  for (index_t i = 0; i < old_capacity; ++i) {
    if (cell->hash != 0) {
      void* key = _cell_key(cell);
      void* value = _cell_value(m, cell);
      map_write_hash((HMap)m, key, value, cell->hash);
    }

    cell = (Map_Cell*)((byte*)cell + m->cell_size);
  }
}

// Helper to check if the map needs to be expanded, and copy values over if so.
static bool _map_check_expand(Map_Internal* m, index_t new_size) {
  index_t cap75 = (m->capacity >> 2) + (m->capacity >> 1);

  // Don't need to expand if the new size is under 75% of capacity
  if (new_size < cap75) {
    return FALSE;
  }

  // Handle the case for our first allocation
  // TODO: is this redundant after adding _map_get_slot_init?
  if (!m->data) {
    _map_initialize(m, new_size);
    return TRUE;
  }

  // TODO: If we're set to not expand, don't

  byte* old_data = m->data;
  index_t old_capacity = m->capacity;
  _map_initialize(m, MAX(new_size, m->capacity));
  _map_clone(m, old_data, old_capacity);
  free(old_data);
  return TRUE;
}

HMap imap_new
( index_t key_size
, index_t element_size
, compare_fn compare
, hash_fn hash
) {
  Map_Internal* ret = malloc(sizeof(Map_Internal));
  assert(ret);

  // Ensure the key/value pair is large enough to sub for the free_prev pointer.
  index_t pair_size = key_size + element_size;
  if ((size_t)pair_size < sizeof(void*)) pair_size = sizeof(void*);

  *ret = (Map_Internal) {
    .size = 0,
    .capacity = 0,
    .key_size = key_size,
    .element_size = element_size,
    .cell_size = sizeof(Map_Cell) + pair_size - sizeof(void*),
    .compare_keys = compare,
    .hash_key = hash,
    .free_list = NULL,
    .data = NULL,
  };
  return (HMap)ret;
}

void map_callback_dtor(HMap m_in, delete_fn del_key, delete_fn del_value) {
  HMAP_INTERNAL;
  m->delete_key = del_key;
  m->delete_value = del_value;
}

HMap map_copy(HMap m_in) {
  //HMAP_INTERNAL;
  (void)m_in;
  return NULL;
}

void map_reserve(HMap m_in, index_t capacity) {
  HMAP_INTERNAL;
  assert(capacity >= 0);
  if (capacity <= m->capacity) return;

  if (!m->data) {
    _map_initialize(m, capacity);
  }
  else {
    void* old_data = m->data;
    index_t old_capacity = m->capacity;
    _map_initialize(m, capacity);
    _map_clone(m, old_data, old_capacity);
    free(old_data);
  }
}

void map_delete(HMap* m_in) {
  if (!m_in || !*m_in) return;
  Map_Internal* m = *(Map_Internal**)m_in;
  free(m->data);
  free(m);
  *m_in = NULL;
}

void map_clear(HMap m_in) {
  HMAP_INTERNAL;
  _map_clear(m);
  m->size = 0;
}

void map_free(HMap m_in) {
  HMAP_INTERNAL;
  free(m->data);
  m->size = 0;
  m->capacity = 0;
  m->free_list = NULL;
  m->data = NULL;
}

res_ensure_t map_ensure_hash(HMap m_in, const void* key, hash_t hash) {
  HMAP_INTERNAL;
  assert(key);
  assert(hash == _key_hash(m, key)); // redundant, ensure correct hashes in test

  Map_Cell* slot = _map_get_slot_init(m, hash);

  // if the slot is occupied, check if our value is already in the map
  if (slot->hash) {
    Map_Cell* cell = _cell_search_bucket(m, slot, key, hash);

    if (cell) {
      return (res_ensure_t) {
        .value = _cell_value(m, cell),
        .is_new = FALSE,
      };
    }
  }

  // TODO: if the map capacity is locked, check if we've reached capacity
  if (/*locked && */ m->size >= m->capacity) {
    return (res_ensure_t) { .value = NULL, .is_new = FALSE };
  }

  // if our key is not already in the map, check expansion and update slot
  if (_map_check_expand(m, m->size + 1)) {
    slot = _map_get_slot(m, hash);
  }

  // if the slot is occupied, check if the occupant actually belongs there
  if (slot->hash) {
    Map_Cell* cell = _map_get_slot(m, slot->hash);

    if (slot != cell) {
      _cell_move(m, _cell_take_from_free_list(m), slot);
      slot->bucket_next = slot;
    }

    // the other cell does belong here, so add the new key to its bucket
    else {
      cell = _cell_take_from_free_list(m);
      cell->bucket_next = slot->bucket_next;
      slot->bucket_next = cell;
      slot = cell;
    }
  }

  // if the slot was unoccupied, we only have to remove it from the free list
  else {
    _cell_remove_from_free_list(m, slot);
    slot->bucket_next = slot; // start of bucket ring list
  }

  // with the correct slot in hand, update the key and return
  ++m->size;
  slot->hash = hash;
  memcpy(_cell_key(slot), key, m->key_size);

  return (res_ensure_t) {
    .value = _cell_value(m, slot),
    .is_new = TRUE,
  };
}

res_ensure_t map_ensure(HMap m_in, const void* key) {
  HMAP_INTERNAL;
  assert(key);
  return map_ensure_hash(m_in, key, _key_hash(m, key));
}

void* map_emplace_hash(HMap m_in, const void* key, hash_t hash) {
  res_ensure_t result = map_ensure_hash(m_in, key, hash);
  if (!result.is_new) return NULL;
  return result.value;
}

void* map_emplace(HMap m_in, const void* key) {
  res_ensure_t result = map_ensure(m_in, key);
  if (!result.is_new) return NULL;
  return result.value;
}

bool map_write_hash(HMap m_in, const void* key, const void* val, hash_t hash) {
  res_ensure_t result = map_ensure_hash(m_in, key, hash);
  memcpy(result.value, val, m_in->element_size);
  return result.is_new;
}

bool map_write(HMap m_in, const void* key, const void* value) {
  res_ensure_t result = map_ensure(m_in, key);
  memcpy(result.value, value, m_in->element_size);
  return result.is_new;
}

bool map_insert_hash(HMap m_in, const void* key, const void* val, hash_t hash) {
  res_ensure_t result = map_ensure_hash(m_in, key, hash);
  if (!result.is_new) return FALSE;
  memcpy(result.value, val, m_in->element_size);
  return TRUE;
}

bool map_insert(HMap m_in, const void* key, const void* value) {
  res_ensure_t result = map_ensure(m_in, key);
  if (!result.is_new) return FALSE;
  memcpy(result.value, value, m_in->element_size);
  return TRUE;
}

void* map_ref_hash(HMap m_in, const void* key, hash_t hash) {
  HMAP_INTERNAL;
  assert(key);
  assert(hash);
  Map_Cell* slot = _map_get_slot(m, hash);
  if (!slot || !slot->hash) return NULL;
  Map_Cell* cell = _cell_search_bucket(m, slot, key, hash);
  if (!cell) return NULL;
  return _cell_value(m, cell);
}

void* map_ref(HMap m_in, const void* key) {
  HMAP_INTERNAL;
  assert(key);
  hash_t hash = _key_hash(m, key);
  return map_ref_hash(m_in, key, hash);
}

pair_kv_t map_next(HMap m_in, const void* key) {
  HMAP_INTERNAL;
  if (!m->size) return (pair_kv_t) { NULL, NULL };

  byte* const data_end = m->data + m->capacity * m->cell_size;

  // get address of next slot in the map
  byte* bkey = (byte*)key; // the map owns the pointer, const cast is fine

  if (bkey) {
    assert(bkey >= m->data);
    bkey += m->cell_size;
  } else {
    bkey = &((Map_Cell*)m->data)->key_start;
  }

  // get cell for that key
  Map_Cell* cell = (Map_Cell*)(bkey - offsetof(Map_Cell, key_start));

  // find the next occupied cell
  while ((byte*)cell < data_end) {
    if (cell->hash) {
      return (pair_kv_t) {
        .key = &cell->key_start,
        .value = _cell_value(m, cell),
      };
    }
    cell = (Map_Cell*)((byte*)cell + m->cell_size);
  }

  return (pair_kv_t) { NULL, NULL };
}

bool map_remove_hash(HMap m_in, const void* key, hash_t hash) {
  HMAP_INTERNAL;
  assert(key);
  assert(hash);

  Map_Cell* slot = _map_get_slot(m, hash);
  if (!slot || !slot->hash) return FALSE;

  Map_Cell* cell = _cell_search_bucket(m, slot, key, hash);
  if (!cell) return FALSE;

  if (m->delete_key) {
    m->delete_key(&(void*){ _cell_key(cell) });
  }

  if (m->delete_value) {
    m->delete_value(&(void*){ _cell_value(m, cell) });
  }

  // if this is the last item in the bucket, free the slot
  if (cell == slot && slot->bucket_next == slot) {
    _cell_move_to_free_list(m, slot);
  }

  // if this in the main slot but there are others in the bucket, shift one here
  else if (cell == slot) {
    Map_Cell* next = slot->bucket_next;
    _cell_move(m, slot, next);
    _cell_move_to_free_list(m, next);
  }

  // if there are other items in the bucket, but this isn't the main slot
  else {
    Map_Cell* prev = _cell_bucket_end(cell);
    prev->bucket_next = cell->bucket_next;
    _cell_move_to_free_list(m, cell);
  }

  --m->size;
  return TRUE;
}

bool map_remove(HMap m_in, const void* key) {
  HMAP_INTERNAL;
  assert(key);
  hash_t hash = _key_hash(m, key);
  return map_remove_hash(m_in, key, hash);
}

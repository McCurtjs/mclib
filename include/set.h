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

#ifndef MCLIB_HASHSET_H_
#define MCLIB_HASHSET_H_

//
// Dynamic Set container
//
// // Create, Setup, Delete
// Map_K_V      set_v_new();
// Map_K_V      set_v_new_reserve(index_t capacity);
// void         set_v_reserve(Map_K_V, index_t capacity);
// void         set_v_clear(Map_K_V);
// void         set_v_free(Map_K_V);
// void         set_v_delete(Map_K_V*);
//
// // Item Addition
// res_ensure_t set_v_emplace(Set_V, const V* value);
// void         set_v_write(Map_K_V, const V* value);
// bool         set_v_insert(Map_K_V, const V* value);
// bool         set_v_add(Map_K_V, V value);
//
// // Item Removal
// bool         set_v_remove(Map_K_V, const V* value);
// bool         set_v_erase(Map_K_V, V value);
//
// // Accessors
// bool         set_v_contains(Map_K_V, const V* value);
// const V*     set_v_ref(Map_K_V, const V* value);
// bool         set_v_has(Map_K_V, V value);
// V            set_v_get(Map_K_V, V value);
// V            set_v_get_or_default(Map_K_V, V value, V default);
// bool         set_v_read_or_default(Map_K_V, key, V* out, V default);
//

#include "types.h"

typedef struct _opaque_Set_base_t {
  index_t CONST size;
  index_t CONST capacity;
  index_t CONST element_size;
  bool          fixed_size;
}* HSet; // Set_void? Set_base?

typedef struct set_ensure_t {
  const void* value;
  bool        is_new;
} set_ensure_t;

typedef void (*set_process_fn)(void* value);

#define       set_new(TYPE, FN_HASH, FN_CMP) \
                iset_new(sizeof(TYPE), FN_HASH, FN_CMP)

//void*       set_emplace_hash(HSet set, const void* key, hash_t hash);

HSet         iset_new(index_t element_size, hash_fn hash, compare_fn cmp);
void          set_callbacks_element(HSet, copy_fn elem_cpy, delete_fn elem_del);
//void        set_callback_copy(HSet set);
//void        set_callback_move(HSet set);
HSet          set_copy(HSet to_copy);
void          set_reserve(HSet, index_t capacity);
void          set_delete(HSet*);
void          set_clear(HSet);
void          set_free(HSet);

set_ensure_t  set_ensure(HSet, const void* value);
bool          set_write(HSet, const void* value);
bool          set_insert(HSet, const void* value);
const void*   set_ref(HSet, const void* value);
bool          set_contains(HSet, const void* value);
bool          set_read_or_default(
                HSet, const void* value, void* out, const void* default_value);
const void*   set_next(HSet, const void* iterator);
bool          set_remove(HSet, const void* value);

#define set_foreach_index(VALUE, INDEX, SET)                                  \
  VALUE = NULL;                                                               \
  for (index_t INDEX = 0; (VALUE = set_next((HSet)(SET), (VALUE))); ++INDEX)  //

#define set_foreach(VALUE, SET)                                               \
  VALUE = NULL;                                                               \
  while ( (VALUE = set_next((HSet)(SET), (VALUE))) )                         //

#endif

// Specialized container/template type
#ifdef con_type

#ifdef con_prefix
# define _set_type MACRO_CONCAT(HSet_, con_prefix)
# define _prefix(_FN) MACRO_CONCAT3(set_, con_prefix, _FN)
#else
# define _set_type MACRO_CONCAT(HSet_, con_type)
# define _prefix(_FN) MACRO_CONCAT3(set_, con_type, _FN)
#endif
#define _ensure_type _prefix(_ensure_t)

#ifndef con_type_hash_compare
# ifdef con_cmp
#   define _value_cmp MACRO_CONCAT3(_cmp_, _set_type, _fn)
static int _value_cmp(const void* lhs, const void* rhs, size_t elem_size) {
  assert(elem_size == sizeof(con_type));
  return key_cmp((const con_type*)lhs, (const con_type*)rhs, elem_size);
}
# else
#   define _value_cmp NULL
# endif

# ifdef con_hash
#   define _value_hash MACRO_CONCAT3(_hash_, _set_type, _fn)
static hash_t _value_hash(const void* element, index_t element_size) {
  assert(element_size == sizeof(con_type));
  return con_hash((const con_type*)element, element_size);
}
# else
#   define _value_hash NULL
# endif
#endif

typedef struct MACRO_CONCAT3(_opaque_, _set_type, _base_t) {
  index_t const size;
  index_t const capacity;
  index_t const element_size;
  bool          fixed_size;
}* _set_type;

typedef union _ensure_type {
  set_ensure_t base;
  struct {
    const con_type* value;
    bool is_new;
  };
} _ensure_type;

// \brief Initializes a hashset of the given type. Allocates no new space for
//    the array contents until an item is added or space is reserved.
//
// \returns A new empty hashset, ready for use.
static inline _set_type _prefix(_new)
(void) {
#ifdef con_type_hash_compare
  HSet ret = iset_new(sizeof(con_type), con_type_hash_compare);
#else
  HSet ret = set_new(con_type, _value_hash, _value_cmp);
#endif

#ifdef con_type_copy_delete
  set_callbacks_element(ret, con_type_copy_delete);
#endif

  return (_set_type)ret;
}

// \brief Reserves space in the hashset. The actual reserved space will be a
//    power of two above the given value.
//
// \param capacity - the number of elements to reserve spcae for
static inline void _prefix(_reserve)
(_set_type set, index_t capacity) {
  set_reserve((HSet)set, capacity);
}

// \brief Initializes a new set of the given type. Pre-allocates space for at
//    least N elements. The resulting capacity will be a power of 2 that can
//    contain the desired element count.
//
// \param capacity - the number of elements to reserve space for
//
// \returns A new empty hashset with the given capacity.
static inline _set_type _prefix(_new_reserve)
(index_t capacity) {
  _set_type ret = _prefix(_new)();
  _prefix(_reserve)(ret, capacity);
  return ret;
}

// \brief Deletes the set and its contents. Once deleted, the provided pointer
//    reference will be nulled.
static inline void _prefix(_delete)
(_set_type* set) {
  set_delete((HSet*)set);
}

// \brief Performs a soft-delete of the set contents without changing capacity.
static inline void _prefix(_clear)
(_set_type set) {
  set_clear((HSet)set);
}

// \brief Frees the contents of the set, but retains the base object for re-use.
static inline void _prefix(_free)
(_set_type set) {
  set_free((HSet)set);
}

// \brief Finds a value already in the set or inserts a new copy if it wasn't
//    already present.
//
// \param value - the value to ensure is in the set.
//
// \returns a map_ensure_t struct variant with:
//    - value:  a pointer to the canon value in the set.
//    - is_new: a boolean indicating whether or not the value was newly added.
static inline _ensure_type _prefix(_ensure)
(_set_type set, const con_type* value) {
  set_ensure_t ret = set_ensure((HSet)set, value);
  return (_ensure_type) { .base = ret };
}

// \brief Writes a copy of the given value into the set. The value will be
//    inserted if not already present, or have its canonical representation
//    overwritten.
//
// \param value - the value to insert or replace.
static inline bool _prefix(_write)
(_set_type set, const con_type* value) {
  return set_write((HSet)set, value);
}

static inline bool _prefix(_replace)
(_set_type set, con_type value) {
  return set_write((HSet)set, &value);
}

// \brief Inserts a copy of the given element into the given set position. The
//    value will only be written if the key is a new addition to the set - if
//    there is a conflict, no action will be taken and it will return false.
//
// \param key - the location in the set to write the element to
//
// \param value - the element to attempt to copy into the set.
//
// \returns A boolean value indicating whether or not a value was written.
static inline bool _prefix(_insert)
(_set_type set, const con_type* value) {
  return set_insert((HSet)set, value);
}

static inline bool _prefix(_add)
(_set_type set, con_type value) {
  return set_insert((HSet)set, &value);
}

// \brief Removes a value from the set. If not found, no action is performed.
//
// \param value - the value to remove from the set
//
// \returns true if an element was removed, false otherwise
static inline bool _prefix(_remove)
(_set_type set, const con_type* value) {
  return set_remove((HSet)set, value);
}

static inline bool _prefix(_erase)
(_set_type set, con_type value) {
  return set_remove((HSet)set, &value);
}

// \brief Returns a reference to the element at the given set position, or NULL
//    if the key is not contained in the set.
//
// \param key - the location in the set to retrieve an element from
//
// \returns a pointer to the element at the given key, or NULL if none is found.
static inline const con_type* _prefix(_ref)
(_set_type set, const con_type* value) {
  return set_ref((HSet)set, value);
}

// \brief Gets an equivalent copy of the given value in the set. If the value
//    is not present, an assert will be thrown.
//
// \param value - the location in the set to retrieve the element from
//
// \returns A copy of the element in the set.
static inline con_type _prefix(_get)
(_set_type set, con_type value) {
  assert(set);
  const con_type* element = set_ref((HSet)set, &value);
  assert(element);
  return *element;
}

// \brief Gets a copy of the element at the given key position. If the location
//    is not present in the set the provided default value will be used instead.
//
// \param key - the location in the set to retrieve an element from
//
// \param default_value - the value to return if no element is found in the set
//
// \returns The value at the key position in the set if found, or the default.
static inline con_type _prefix(_get_or_default)
(_set_type set, con_type value, con_type default_value) {
  assert(set);
  const con_type* element = set_ref((HSet)set, &value);
  if (!element) return default_value;
  return *element;
}

static inline bool _prefix(_contains)
(_set_type set, const con_type* value) {
  return set_contains((HSet)set, value);
}

static inline bool _prefix(_has)
(_set_type set, con_type value) {
  return set_contains((HSet)set, &value);
}

// \brief Given a valid pointer to an iterator/key within the set, returns the
//    next slot in the set in memory order. Used to iterate over all elements.
//
// \param iterator - a key value previously returned from this function, or
//    NULL to get the first element in the set.
//
// \returns a pair containing pointers to a key and its value within the set.
static inline const con_type* _prefix(_next)
(_set_type set, const con_type* iterator) {
  return set_next((HSet)set, iterator);
}

// \brief Copies the canonical value into the out parameter if it's present
//    in the set, otherwise copies the provided default value instead.
//
// \param value - the location in the set to read the item from
//
// \param out_element - a pointer to the object to copy the data into
//
// \param default_value - the value to write if the value wasn't in the set
//
// \returns True if the written item is from the set, false if default is used.
static inline bool _prefix(_read_or_default)
( _set_type set
, const con_type* value
, con_type* out_element
, const con_type* default_value
) {
  assert(set);
  assert(out_element);
  const con_type* canon = set_ref((HSet)set, &value);
  if (!canon) {
    *out_element = *default_value;
    return false;
  }
  *out_element = *canon;
  return true;
}

#undef _set_type
#undef _prefix
#undef _ensure_type
#undef _value_cmp

#endif

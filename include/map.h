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

#ifndef MCLIB_HASHMAP_H_
#define MCLIB_HASHMAP_H_

//
// Dynamic Hashmap container
//
// // Create, Setup, Delete
// Map_K_V      map_k_v_new();
// Map_K_V      map_k_v_new_reserve(index_t capacity);
// void         map_k_v_reserve(Map_K_V, index_t capacity);
// void         map_k_v_clear(Map_K_V);
// void         map_k_v_free(Map_K_V);
// void         map_k_v_delete(Map_K_V*);
//
// // Item Addition
// res_ensure_t map_k_v_ensure(Map_K_V, key); { .value, .is_new }
// V*           map_k_v_emplace(Map_K_V, key);
// void         map_k_v_write(Map_K_V, key, value);
// bool         map_k_v_insert(Map_K_V, key, value);
//
// // Item Removal
// bool         map_k_v_remove(Map_K_V, key);
//
// // Accessors
// V*           map_k_v_ref(Map_K_V, key);
// V            map_k_v_get(Map_K_V, key);
// V            map_k_v_get_or_default(Map_K_V, key, V default);
// bool         map_k_v_read(Map_K_V, key, V* out);
// bool         map_k_v_read_or_default(Map_K_V, key, V* out, V default);
// bool         map_k_v_contains_key(Map_K_V, key);
//

#include "types.h"

typedef int (*compare_fn)(const void* lhs, const void* rhs);
typedef hash_t (*hash_fn)(const void* key);
typedef void (*delete_fn)(void** to_delete);

typedef struct _opaque_Map_base {
  index_t const size;
  index_t const capacity;
  index_t const key_size;
  index_t const element_size;
  bool          fixed_size;
}* HMap; // Map_void? Map_base?

typedef struct res_ensure_t {
  void* value;
  bool is_new;
} res_ensure_t;

#define       map_new(T_KEY, T_VAL, FN_CMP, FN_HASH)  \
             imap_new(sizeof(T_KEY), sizeof(T_VAL), FN_CMP, FN_HASH)

//void*     map_emplace_hash(HMap map, const void* key, hash_t hash);

HMap         imap_new(index_t ksz, index_t vsz, compare_fn cmp, hash_fn hash);
void          map_callback_dtor(HMap map, delete_fn key_del, delete_fn val_del);
//void        map_callback_copy(HMap map);
//void        map_callback_move(HMap map);
HMap          map_copy(HMap to_copy);
void          map_reserve(HMap map, index_t capacity);
void          map_delete(HMap* map);
void          map_free(HMap map);
void          map_clear(HMap map);

res_ensure_t  map_ensure(HMap map, const void* key);
void*         map_emplace(HMap map, const void* key);
void          map_write(HMap map, const void* key, const void* value);
bool          map_insert(HMap map, const void* key, const void* value);
void*         map_ref(HMap map, const void* key);
bool          map_remove(HMap map, const void* key);
//bool        map_read(HMap map, key_t key, void* out_element);
//bool        map_contains_element(HMap map, const void* to_find);
//bool        map_contains_key(HMap map, key_t key);

#endif

// Specialized container/template type
#ifdef con_type

#ifdef con_prefix
# define _map_type MACRO_CONCAT(HMap_, con_prefix)
# define _prefix(_FN) MACRO_CONCAT3(map_, con_prefix, _FN)
# define _ensure_type MACRO_CONCAT3(res_ensure_, con_prefix, _t)
#else
# define _ensure_type MACRO_CONCAT3(res_ensure_, con_type, _t)
# ifdef key_type
#   define _map_type MACRO_CONCAT4(HMap_, key_type, _, con_type)
#   define _prefix(_FN) MACRO_CONCAT5(map_, key_type, _, con_type, _FN)
# else
#   define _map_type MACRO_COMCAT(HMap_, con_type)
#   define _prefix(_FN) MACRO_CONCAT3(map_, con_type, _FN)
# endif
#endif

// By default, if no other key type was defined, use string slices.
// When using string slices, the "slice" type will be omitted from the type name.
//    ex: - A map of string slices to String objects will be `HMap_String`
//        - A map of string slices to Moop objects would be `HMap_Moop`
//        - A map using Strings to Moop objects would be `HMap_String_Moop`
#ifdef key_type
# define _key_type key_type
#else
# include "slice.h"
# define _key_type slice_t
# define _con_cmp slice_compare_vptr
# define _con_hash slice_hash_vptr
#endif

#ifndef _con_cmp
# ifdef con_cmp
#   define _con_cmp MACRO_CONCAT3(_cmp_, _map_type, _fn)
static int _con_cmp(const void* lhs, const void* rhs) {
  return con_cmp(lhs, rhs);
}
# else
#   define _con_cmp NULL
# endif
#endif

#ifndef _con_hash
# ifdef con_hash
#   define _con_hash MACRO_CONCAT3(_hash_, _map_type, _fn)
static hash_t _con_hash(const void* key) {
  return con_hash(key);
}
# else
#   define _con_hash NULL
# endif
#endif

typedef struct _ensure_type {
  con_type* value;
  bool is_new;
} _ensure_type;

typedef struct MACRO_CONCAT3(_opaque_, _map_type, _base) {
  index_t const size;
  index_t const capacity;
  index_t const key_size;
  index_t const element_size;
  bool          fixed_size;
}* _map_type;

// \brief Initializes a hashmap of the given type. Allocates no new space for
//    the array contents until an item is added or space is reserved.
//
// \returns A new empty hashmap, ready for use.
static inline _map_type _prefix(_new)
(void) {
  HMap ret = map_new(con_type, _key_type, _con_cmp, _con_hash);

#if defined(con_type_dtor) || defined(key_type_dtor)
  delete_fn delete_key = NULL;
  delete_fn delete_value = NULL;

# ifdef con_type_dtor
  delete_value = (void*)con_type_dtor;
# endif
# ifdef key_type_dtor
  delete_key = (void*)key_type_dtor;
# endif

  if (delete_key || delete_value) map_callback_dtor(delete_key, delete_value);
#endif

  return (_map_type)ret;
}

// \brief Initializes a new map of the given type. Pre-allocates space for at
//    least N elements. The resulting capacity will be a power of 2 that can
//    contain the desired element count.
//
// \param capacity - the number of elements to reserve space for
//
// \returns A new empty hashmap with the given capacity.
static inline _map_type _prefix(_new_reserve)
(index_t capacity) {
  HMap ret = map_new(con_type, _key_type, _con_cmp, _con_hash);
  map_reserve(ret, capacity);
  return (_map_type)ret;
}

// \brief Reserves space in the hashmap. The actual reserved space will be a
//    power of two above the given value.
//
// \param capacity - the number of elements to reserve spcae for
static inline void _prefix(_reserve)
(_map_type map, index_t capacity) {
  map_reserve((HMap)map, capacity);
}

// \brief Performs a soft-delete of the map contents without changing capacity.
static inline void _prefix(_clear)
(_map_type map) {
  map_clear((HMap)map);
}

// \brief Frees the contents of the map, but retains the base object for re-use.
static inline void _prefix(_free)
(_map_type map) {
  map_free((HMap)map);
}

// \brief Deletes the map and its contents. Once deleted, the provided pointer
//    reference will be nulled.
static inline void _prefix(_delete)
(_map_type* map) {
  map_delete((HMap*)map);
}

// \brief Finds an existing element or adds the space for it if it isn't already
//    present. The value may or may not contain an already existing value, which
//    can be determined by the `is_new` value in the resulting res_ensure_t
//    struct.
//
// \param key - the key for the location to either find or add to the map
//
// \returns an res_ensure_t struct variant with:
//    - value: a pointer to the location represented by the given key. May be
//              either a valid value, or newly allocated and uninitialized.
//    - is_new: a boolean value indicating whether or not the value was newly
//              added to the map, or if it is already existing valid data.
static inline _ensure_type _prefix(_ensure)
(_map_type map, _key_type key) {
  res_ensure_t ret = map_ensure((HMap)map, &key);
  return *((_ensure_type*)&ret);
}

// \brief Inserts space for an element in the map and returns a pointer to it
//    without performing any initialization or copying into the value.
// 
// \brief Note: Empalce works similar to Insert, in that it will only return a
//    new map location if the key isn't already present. If there's a conflict,
//    no new memory will be assigned for this key. For `get_or_emplace` behavior
//    you can use `map_ensure`.
//
// \param key - the key for the memory's location in the map
//
// \returns A pointer to the newly added and uninitialized element, or NULL if
//    the key was already present in the map.
static inline con_type* _prefix(_emplace)
(_map_type map, _key_type key) {
  return map_emplace((HMap)map, &key);
}

// \brief Writes a copy of the given element into the given position in the map.
//    The value will be written whether or not a value is already present at the
//    given location.
//
// \param key - the location in the map to write the element to
//
// \param value - the element to copy into the map.
static inline void _prefix(_write)
(_map_type map, _key_type key, con_type value) {
  map_write((HMap)map, &key, &value);
}

// \brief Inserts a copy of the given element into the given map position. The
//    value will only be written if the key is a new addition to the map - if
//    there is a conflict, no action will be taken and it will return false.
//
// \param key - the location in the map to write the element to
//
// \param value - the element to attempt to copy into the map.
//
// \returns A boolean value indicating whether or not a value was written.
static inline bool _prefix(_insert)
(_map_type map, _key_type key, con_type value) {
  return map_insert((HMap)map, &key, &value);
}

// \brief Removes an element from the map at the given key location. If the key
//    does not match any in the map, no action is performed.
//
// \param key - the key location to remove
//
// \returns true if an element was removed, false otherwise
static inline bool _prefix(_remove)
(_map_type map, _key_type key) {
  return map_remove((HMap)map, &key);
}

// \brief Returns a reference to the element at the given map position, or NULL
//    if the key is not contained in the map.
//
// \param key - the location in the map to retrieve an element from
//
// \returns A pointer to the element at the given key, or NULL if none is found.
static inline con_type* _prefix(_ref)
(_map_type map, _key_type key) {
  return map_ref((HMap)map, &key);
}

// \brief Gets a copy of the element at the given position. If the location of
//    the key is not valid, an assert will be thrown.
//
// \param key - the location in the map to retrieve the element from
//
// \returns A copy of the element in the map.
static inline con_type _prefix(_get)
(_map_type map, _key_type key) {
  assert(map);
  con_type* element = map_ref((HMap)map, &key);
  assert(element);
  return *element;
}

// \brief Gets a copy of the element at the given key position. If the location
//    is not present in the map the provided default value will be used instead.
//
// \param key - the location in the map to retrieve an element from
//
// \param default_value - the value to return if no element is found in the map
//
// \returns The value at the key position in the map if found, or the default.
static inline con_type _prefix(_get_or_default)
(_map_type map, _key_type key, con_type default_value) {
  assert(map);
  con_type* element = map_ref((HMap)map, &key);
  if (!element) return default_value;
  return *element;
}

// \brief Copies the value at the given key position into the output element. If
//    the key is not present in the map, nothing is written.
//
// \param key - the location in the map to read the item from
//
// \param out_element - a pointer to the object to copy the data into
//
// \returns True if an element was found and written, false otherwise.
static inline bool _prefix(_read)
(_map_type map, _key_type key, con_type* out_element) {
  assert(map);
  assert(out_element);
  con_type* value = map_ref((HMap)map, &key);
  if (!value) return false;
  *out_element = *value;
  return true;
}

// \brief Copies the value at the given key position into the output element. If
//    the key is not present, the default value will be copied instead.
//
// \param key - the location in the map to read the item from
//
// \param out_element - a pointer to the object to copy the data into
//
// \param default_value - the value to write if the key wasn't in the map
//
// \returns True if the written item is from the map, false if default is used.
static inline bool _prefix(_read_or_default)
(_map_type map, _key_type key, con_type* out_element, con_type default_value) {
  assert(map);
  assert(out_element);
  con_type* value = map_ref((HMap)map, &key);
  if (!value) {
    *out_element = default_value;
    return false;
  }
  *out_element = *value;
  return true;
}

// \brief Checks if a given key is contained in the map.
//
// \param key - the key to check for
//
// \returns True if the map contains the key, false otherwise.
static inline bool _prefix(_contains_key)
(_map_type map, _key_type key) {
  return map_ref((HMap)map, &key);
}

#undef _map_type
#undef _key_type
#undef _prefix
#undef _ensure_type
#undef _con_cmp
#undef _con_hash

#endif

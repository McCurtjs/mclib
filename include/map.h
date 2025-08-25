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

//
// Dynamic Hashmap container
// 
// 
// 
// // Create, Setup, Delete
// Map_T    map_t_new();
// Map_T    map_t_new_reserve(index_t capacity)
// void     map_t_reserve(index_t capacity)
// 
//

#ifndef MCLIB_HASHMAP_H_
#define MCLIB_HASHMAP_H_

#include "types.h"

typedef size_t key_t;
typedef size_t hash_t;
typedef int (*key_compare_fn)(const void* lhs, const void* rhs);
typedef hash_t (*key_hash_fn)(const void* key);

typedef struct inlist_node_t {
  struct inlist_node_t* next;
  struct inlist_node_t* prev;
} inlist_node_t;

typedef struct inlist_node_const_t {
  const struct inlist_node_const_t* next;
  const struct inlist_node_const_t* prev;
} inlist_node_const_t;

typedef union inlist_t {
  inlist_node_t* head;
  const inlist_node_const_t* head_const;
} inlist_t;

typedef struct inlist_const_t {
  const inlist_node_const_t* head_const;
} inlist_c;

typedef struct keylist_t {
  const inlist_node_const_t node;
  const hash_t hash;
} keylist_t;

typedef struct {
  const index_t size;
  const index_t capacity;
  const index_t key_size;
  const index_t element_size;

  union {
    const keylist_t* const items;
    const keylist_t* const keys;
  };
}* HMap; // Map_void? Map_base?

/*
#define map_new(T_KEY, T_VAL) _map_new(sizeof(T_KEY), sizeof(T_VAL))
#define map_new_reserve(T_KEY, T_VAL, capacity) \
                        _map_new_reserve(sizeof(T_KEY), sizeof(T_VAL), capacity)
HMap _map_new(index_t key_size, index_t element_size);
HMap _map_new_reserve(index_t key_size, index_t element_size, index_t capacity);
HMap map_copy(HMap to_copy);
void map_reserve(HMap map, index_t capacity);
void map_clear(HMap map);
void map_free(HMap map);
void map_delete(HMap* map);
void map_write(HMap map, key_t key, const void* in_element);
void* map_emplace(HMap map, key_t key);
bool map_remove(HMap map, key_t key);
void* map_ref(HMap map, key_t key);
bool map_read(HMap map, key_t key, void* out_element);
bool map_contains_element(HMap map, const void* to_find);
bool map_contains_key(HMap map, key_t key);
//*/

#endif


// Specialized container/template type
#ifdef con_type

#ifdef con_prefix
# define _map_type MACRO_CONCAT(HMap_, con_prefix)
# define _prefix(_FN) MACRO_CONCAT3(map_, con_prefix, _FN)
#else
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
# define _kv_type MACRO_CONCAT5(kv_, _key_type, _, con_type, _t)
# define _kv_list_type MACRO_CONCAT5(inlist_kv_, _key_type, _, con_type, _t)
# define _inlist_type MACRO_CONCAT3(inlist_, _key_type, _t)
#else
# include "slice.h"
# define _key_type slice_t
# define _kv_type MACRO_CONCAT3(kv_span_, con_type, _t)
# define _kv_list_type MACRO_CONCAT3(inlist_kv_, con_type, _t)
# define _inlist_type inlist_slice_t
#endif

//#define _map_type MACRO_CONCAT4(HMap_, key_type, _, con_type)
//#define _kv_type MACRO_CONCAT5(kv_, _key_type, _, con_type, _t)
//#define _kv_list_type MACRO_CONCAT5(inlist_kv_, _key_type, _, con_type, _t)

//#define _inlist_type MACRO_CONCAT3(inlist_, _key_type, _t)

typedef struct _inlist_type {
  union {
    inlist_t inlist;
    struct {
      const struct _inlist_type* const next;
      const struct _inlist_type* const prev;
      const hash_t hash;
      const _key_type key;
    };
  };
} _inlist_type;

typedef struct {
  const hash_t hash;
  const _key_type key;
  con_type value;
} _kv_type;

typedef struct _kv_list_type {
  const _kv_type* const next;
  const _kv_type* const prev;
  union {
    _kv_type kv;
    struct {
      const hash_t hash;
      const _key_type key;
      con_type value;
    };
  };
} _kv_list_type;

typedef struct {
  const index_t size;
  const index_t capacity;
  const index_t key_size;
  const index_t element_size;

  union {
    const keylist_t* const keys_inlist;
    const _inlist_type* const keys;
    const _kv_list_type* const items;
  };
}*_map_type;



#undef _map_type
#undef _prefix
#undef _key_type
#undef _kv_type
#undef _kv_list_type
#undef _inlist_type

#endif

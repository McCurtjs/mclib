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

#ifndef MCLIB_PACKEDMAP_H_
#define MCLIB_PACKEDMAP_H_

//
// Dynamic Packed SlotMap Container
//
// An indirect-access array that functions similarly to a SlotMap, but where the
// underlying data is guaranteed to remain contiguous, allowing for access,
// insertion, and removal all in O(1) time, O(n) traversal, and the ability to
// use the data in cases where contiguousness is preferred, such as buffers for
// use in graphics.
//
// Type specialization
//  - con_type:       the type the PackedMap contains.
//  - con_prefix:     [optional] overrides the default name of the container.
//  - con_view_type:  [optional] explicitly sets a view type over view_t.
//  - con_cmp:        [optional] a `compare_nosize_fn` that can be used for
//                    operations like sorting (todo)
//
// When defined, the following will be created:
//
// #define con_type A
// #define con_prefix a
//
// #include "view.h"
// #define con_view_type view_a_t
//
// #include "packedmap.h"
// #undef con_type
// #undef con_prefix
// #undef con_view_type
//
// // Create, Setup, Delete
// PackedMap_A  pmap_v_new();
// PackedMap_A  pmap_v_new_reserve(index_t capacity);
// void         pmap_v_reserve(PackedMap_A, index_t capacity);
// void         pmap_v_truncate(PackedMap_A, index_t capacity);
// void         pmap_v_clear(PackedMap_A);
// void         pmap_v_free(PackedMap_A);
// void         pmap_v_delete(PackedMap_A);
//
// // Item Addition
// A*           pmap_v_emplace(PackedMap_A, slotkey_t* out_key);
// slotkey_t    pmap_v_insert(PackedMap_A, const A* element);
// slotkey_t    pmap_v_add(PackedMap_A, A element);
//
// // Accessors
// slotkey_t    pmap_v_key(PackedMap_A, index_t index);
// A            pmap_v_get(PackedMap_A, slotkey_t);
// A*           pmap_v_ref(PackedMap_A, slotkey_t);
// bool         pmap_v_read(PackedMap_A, slotkey_t, A* out);
//
// // Item Removal
// bool         pmap_v_remove(PackedMap_A, slotkey_t);
//
// // Algorithm
// void         pmap_v_sort(PackedMap_A); // TODO
//

#include "types.h"
#include "span_base.h"

typedef struct _opaque_PackedMap_base_t {
  union {
    view_t        const view;
    struct {
      const void* const begin;
      const void* const end;
    };
  };
  index_t         const element_size;
  index_t         const capacity;
  index_t         const size;
  index_t         const size_bytes;
}* PackedMap;

#define     pm_foreach(PMAP) view_foreach((PMAP)->view)
#define     pm_foreach_index(PMAP) view_foreach((PMAP)->view)

#define     pmap_new(TYPE) ipmap_new(sizeof(TYPE))
#define     pmap_new_reserve(T, capacity) ipmap_new_reserve(sizeof(T), capacity)

PackedMap  ipmap_new(index_t element_size);
PackedMap  ipmap_new_reserve(index_t element_size, index_t capacity);
void        pmap_reserve(PackedMap, index_t capacity);
void        pmap_trim(PackedMap);
void        pmap_clear(PackedMap);
void        pmap_free(PackedMap);
void        pmap_delete(PackedMap*);
void*       pmap_emplace(PackedMap, slotkey_t* out_key);
slotkey_t   pmap_insert(PackedMap, const void* element);
slotkey_t   pmap_key(PackedMap, index_t index);
void*       pmap_ref(PackedMap, slotkey_t);
bool        pmap_read(PackedMap, slotkey_t, void* out_element);
bool        pmap_remove(PackedMap, slotkey_t);

#endif

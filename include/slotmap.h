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

#ifndef MCLIB_SLOTMAP_H_
#define MCLIB_SLOTMAP_H_

//
// Dynamic SlotMap Container
//
// A slot-map for generic data (void*) with the option of setup for
// type-specific variants. A slot-map maps slot_key_t keys to reference values
// that are stored in a non-contiguous array but can be accessed, inserted, or
// removed all in O(1) time.
//
// Type specialization
//  - con_type:   the type the slotmap contains.
//  - con_prefix: [optional] overrides the default name of the container.
//
// When defined, the following will be created:
//
// #define con_type A
// #define con_prefix a
// #include "slotmap.h"
// #undef con_type
// #undef con_prefix
//
// // Create, Setup, Delete
// SlotMap_A    smap_a_new();
// SlotMap_A    smap_a_new_reserve(index_t capacity);
// void         smap_a_reserve();
// void         smap_a_trim(SlotMap_A);
// void         smap_a_clear(SlotMap_A);
// void         smap_a_free(SlotMap_A);
// void         smap_a_delete(SlotMap_A*);
//
// // Item Addition
// A*           smap_a_emplace(SlotMap_A, slotkey_t* out_key);
// slotkey_t    smap_a_insert(SlotMap_A, const A* element);
// slotkey_t    smap_a_add(SlotMap_A, A element);
//
// // Accessors
// A            smap_a_get(SlotMap_A, slotkey_t);
// A*           smap_a_ref(SlotMap_A, slotkey_t);
// bool         smap_a_read(SlotMap_A, slotkey_t, A* out);
//
// // Item Removal
// bool         smap_a_remove(SlotMap_A, slotkey_t);

#include "types.h"

typedef struct _opaque_SlotMap_base_t {
  index_t const element_size;
  index_t const capacity;
  index_t const size;
}* SlotMap;

#define   smap_new(TYPE) ismap_new(sizeof(TYPE))
#define   smap_new_reserve(T, capacity) ismap_new_reserve(sizeof(T), capacity)

SlotMap  ismap_new(index_t element_size);
SlotMap  ismap_new_reserve(index_t element_size, index_t capacity);
void      smap_reserve(SlotMap, index_t capacity);
void      smap_trim(SlotMap);
void      smap_clear(SlotMap);
void      smap_free(SlotMap);
void      smap_delete(SlotMap*);
void*     smap_emplace(SlotMap, slotkey_t* out_key);
slotkey_t smap_insert(SlotMap, const void* element);
void*     smap_ref(SlotMap, slotkey_t);
bool      smap_read(SlotMap, slotkey_t, void* out_element);
bool      smap_remove(SlotMap, slotkey_t);

#endif

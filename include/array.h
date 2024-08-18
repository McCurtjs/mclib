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

#ifndef _MCLIB_DYNAMIC_ARRAY_H_
#define _MCLIB_DYNAMIC_ARRAY_H_

#include "types.h"

typedef struct {
  index_s const element_size;
  index_s const capacity;
  index_s const size;
  index_s const size_bytes;
  void* const arr;
}* Array;

#define array_new(TYPE) _array_new_(sizeof(TYPE))
#define array_new_reserve(TYPE, capacity) _array_new_reserve_(sizeof(TYPE), capacity)
Array   _array_new_(index_s elemenet_size);
Array   _array_new_reserve_(index_s element_size, index_s capacity);
void    array_reserve(Array array, index_s capacity);
void    array_truncate(Array array, index_s capacity);
void    array_clear(Array array);
void    array_free(Array array);
void    array_delete(Array* array);
void*   array_release(Array* array);
index_s array_write(Array array, index_s position, const void* in_element);
index_s array_write_back(Array array, const void* in_element);
void*   array_emplace(Array array, index_s position);
void*   array_emplace_back(Array array);
void*   array_emplace_range(Array array, index_s position, index_s count);
void*   array_emplace_back_range(Array array, index_s count);
index_s array_remove(Array array, index_s position);
index_s array_remove_range(Array array, index_s position, index_s count);
index_s array_remove_unstable(Array array, index_s position);
index_s array_pop_back(Array array);
void*   array_ref(Array array, index_s index);
void*   array_ref_front(Array array);
void*   array_ref_back(Array array);
bool    array_read(const Array array, index_s index, void* out_element);
bool    array_read_front(const Array array, void* out_element);
bool    array_read_back(const Array array, void* out_element);
//void    array_sort(Array array, bool (*cmp)(const void* lhs, const void* rhs));
//void*   array_ref_find(Array array, bool (*predicate)(const void* el));
//void    array_filter(Array array, bool (*filter)(const void* el));

// In order to define type-specific continers, include or re-include the header
// after #defining con_type with the desired type, and optionally con_prefix to
// set the prefix type specifier (if not set, the type will be used directly).
//
// When defined, the following will be created (inline, with no overhead):
//
// #define con_type T
// #define con_prefix t
// #include "array.h"
// #undef con_type
// #undef con_prefix
//
// // Create, Setup, Delete
// Array_T  arr_t_new();
// Array_T  arr_t_new_reserve(index_s capacity);
// void     arr_t_reserve(Array_T, index_s capacity);
// void     arr_t_truncate(Array_T, index_s capacity);
// void     arr_t_clear(Array_T);
// void     arr_t_free(Array_T);
// void     arr_t_delete(Array_T*);
// T*       arr_t_release(Array_T*);
//
// // Item Addition
// index_s  arr_t_insert(Array_T, index_s position, T element);
// index_s  arr_t_push_back(Array_T, T element);
// index_s  arr_t_write(Array_T, index_s position, const T* element);
// index_s  arr_t_write_back(Array_T, const T* element);
// T[1]     arr_t_emplace(Array_T, index_s position);
// T[1]     arr_t_emplace_back(Array_T);
// T[n]     arr_t_emplace_range(Array_T, index_s position, index_s count);
// T[n]     arr_t_emplace_back_range(Array_T, index_s count);
//
// // Item Removal
// index_s  arr_t_remove(Array_T, position);
// index_s  arr_t_remove_range(Array_T, position);
// index_s  arr_t_remove_unstable(Array_T, position);
// index_s  arr_t_pop_back(Array_T);
//
// // Accessors
// T        arr_t_get(Array_T, index_s index);
// T        arr_t_get_front(Array_T);
// T        arr_t_get_back(Array_T);
// T*       arr_t_ref(Array_T, index_s index);
// T*       arr_t_ref_front(Array_T);
// T*       arr_t_ref_back(Array_T);
// bool     arr_t_read(Array_T, index_s index, T* out);
// bool     arr_t_read_front(Array_T, T* out);
// bool     arr_t_read_back(Array_T, T* out);
//
// // Algorithm
// void     arr_t_filter(Array_T, predicate);
// void     arr_t_sort(Array_T, compare_fn cmp);
// T        arr_t_find(Array_T, predicate);
// T*       arr_t_ref_find(Array_T, predicate);
//

// \brief A macro shorthand to write foreach loops with any dynamic Array or
//    Array-based sub-types.
//
// \brief usage example:
// \brief MyType* array_foreach(iterator, array) { use(iterator); }
#define array_foreach(VAR, ARRAY)                                             \
  array_foreach_index(VAR, MACRO_CONCAT(_array_iter, __LINE__), ARRAY)        //

// \brief A macro shorthand to write foreach loops with any dynamic Array or
//    Array-based sub-types. Includes a tracking index value as well.
//
// \brief usage example:
// \brief MyType* array_foreach_index(iter, i, array) { other[i] = *iter; }
#define array_foreach_index(VAR, INDEX, ARRAY)                                \
  VAR = (ARRAY)->arr;                                                         \
  assert(sizeof(*VAR) == ARRAY->element_size);                                \
  for (index_s INDEX = 0; INDEX < ARRAY->size; ++INDEX,                       \
    VAR = (void*)((byte*)ARRAY->arr + INDEX * sizeof(*VAR))                   \
  )                                                                           //

// note: using VAR + i*s instead of just ++VAR in order to ensure the loop will
//    continue to work in cases where a resize is performed during iteration.

// TODO: would it be better to also track an offset rather than multiply?

#endif

// specialized container/template type
#ifdef con_type

// Specialized container functions are declared as arr_<prefix>_<fn>
//    ex: - if con_prefix is 'str', you'll get a function arr_str_push_back
//        - if con_prefix is not set, you'll get arr_String_push_back
#ifdef con_prefix
# define _full_prefix MACRO_CONCAT(arr_, con_prefix)
#else
# define _full_prefix MACRO_CONCAT(arr_, con_type)
#endif

// The type of the specialized array class will be Array_<type>.
//    for example: Array_String, Array_Entity, etc.
#define _arr_type MACRO_CONCAT(Array_, con_type)

#define _prefix(_fn) MACRO_CONCAT(_full_prefix, _fn)

// Annoyingly have to redefine the struct to match - if we just typedef the
//    pointer type, it'll happily accept either as equivalent, but the whole
//    point is to prompt type errors.
typedef struct {
  index_s const element_size;
  index_s const capacity;
  index_s const size;
  index_s const size_bytes;
  union {
    con_type* const arr;
    con_type* const buf;
    con_type* const first;
  };
}* _arr_type;

// \brief Initializes a new array of the given type. Allocates no new space for
//    the array contents until an item is added.
//
// \returns A new empty dynamic array, ready for use.
static inline _arr_type _prefix(_new)
(void) {
  return (_arr_type) { (_arr_type)array_new(con_type) };
}

// \brief Initialies a new array of the given type. Pre-allocates space for N
//    elements to be added without needing to expand the array. The array after
//    initialization is still empty.
//
// \param capacity - the number of elements to reserve space for
//
// \returns A new empty dynamic array with the given capacity.
static inline _arr_type _prefix(_new_reserve)
(index_s capacity) {
  return (_arr_type)array_new_reserve(con_type, capacity);
}

// \brief Reserves space in the array so that it can contain at least N
//    elements. This will not reserve space for N _additional_ elements, any
//    items already in the array will still count towards the final capacity.
//
// \param capacity - the number of elements to reserve space for
static inline void _prefix(_reserve)
(_arr_type arr, index_s capacity) {
  array_reserve((Array)arr, capacity);
}

// \brief Truncates the array, may decrease the size of the array and remove
//    elements from the end of the array until the size requirement is met.
//
// \brief For example, can free extra reserved space by calling
//    `arr_type_truncate(array, array.size);`
//
// \brief Performs no operation if the array is already smaller than max_size.
//
// \param max_size - the maximum resulting capacity of the array
static inline void _prefix(_truncate)
(_arr_type arr, index_s capacity) {
  array_truncate((Array)arr, capacity);
}

// \brief Performs a soft-delete of the array contents without changing
//    capacity or freeing any allocations.
static inline void _prefix(_clear)
(_arr_type arr) {
  array_clear((Array)arr);
}

// \brief Frees the contents of the array back to the allocator. The array
//    object itself will still be stored in memory and can still be used.
static inline void _prefix(_free)
(_arr_type arr) {
  array_free((Array)arr);
}

// \brief Deletes the array object and its contents from memory. Once deleted,
//    the provided pointer reference will be nulled.
static inline void _prefix(_delete)
(_arr_type* p_arr) {
  array_delete((Array*)p_arr);
}

// \brief Deletes the array object without erasing the data.
//
// \returns The array without freeing it.
static inline con_type* _prefix(_release)
(_arr_type* p_arr) {
  return array_release((Array*)p_arr);
}

// \brief Inserts a copy of the given element into the given position in the
//    array. Elements after the insert position will be moved one space forward.
//
// \param position - the index at which the new element will be accessed
//
// \param element - the element to insert into the array
//
// \returns The size of the array after adding the element.
static inline index_s _prefix(_insert)
(_arr_type arr, index_s position, con_type element) {
  return array_write((Array)arr, position, &element);
}

// \brief Inserts a copy of the given element into the back of the array
//
// \param element - the element to insert into the array
//
// \returns The size of the array after adding the element.
static inline index_s _prefix(_push_back)
(_arr_type arr, con_type element) {
  return array_write_back((Array)arr, &element);
}

// \brief Inserts a copy of the element referenced by the given pointer into the
//    array at the given position.
//
// \param position - the index at which the new element will be accessed
//
// \param element - a pointer to the element to write into the array
//
// \returns The size of the array after adding the element.
static inline index_s _prefix(_write)
(_arr_type arr, index_s position, const con_type* element) {
  return array_write((Array)arr, position, element);
}

// \brief Inserts a copy of the element referenced by the given pointer into the
//    back of the array.
//
// \param element - a pointer to the element to write into the array
//
// \returns The size of the array after adding the element.
static inline index_s _prefix(_write_back)
(_arr_type arr, const con_type* element) {
  return array_write_back((Array)arr, element);
}

// \brief Inserts space for an element in the array and returns a pointer to it
//    without performing any initialization or copying into the given position.
//
// \param position - the index at which the new element will be accessed
//
// \returns A pointer to the newly added and uninitialized element.
static inline con_type* _prefix(_emplace)
(_arr_type arr, index_s position) {
  return array_emplace((Array)arr, position);
}

// \brief Inserts space for an element at the back of the array and returns a
//    pointer to it without performing any initialization.
//
// \returns A pointer to the newly added and uninitialized element.
static inline con_type* _prefix(_emplace_back)
(_arr_type arr) {
  return array_emplace_back((Array)arr);
}

// \brief Inserts space for count elements in the array and returns a pointer to
//    the first without performing any initialization or copying into the range.
//
// \param position - the index at which the first new element will be accessed.
//
// \param count - the number of elements to make space for
//
// \returns A pointer to the first newly added and uninitialized elements.
static inline con_type* _prefix(_emplace_range)
(_arr_type arr, index_s position, index_s count) {
  return array_emplace_range((Array)arr, position, count);
}

// \brief Inserts space for a number of elements at the back of the array and
//    returns a pointer to the first element allocated this way. No
//    initialization is performed on any of the new elements.
//
// \returns A pointer to the first of the newly added uninitialized elements.
static inline con_type* _prefix(_emplace_back_range)
(_arr_type arr, index_s count) {
  return array_emplace_back_range((Array)arr, count);
}

// \brief Removes the given element in the array, shifting the remaining items
//    to fill the space
//
// \param position - The index to remove
//
// \returns The size of the array after removing the element
static inline index_s _prefix(_remove)
(_arr_type arr, index_s position) {
  return array_remove((Array)arr, position);
}

// \brief Removes the given element in the array, replacing its location in
//    memory with the last element of the array in order to avoid copying the
//    rest of the array elements.
//
// \brief In other words, O(1) removal time, but doesn't maintain element order.
//
// \param position - the index to remove and swap with the last element
//
// \returns The size of the array after removing the element.
static inline index_s _prefix(_remove_unstable)
(_arr_type arr, index_s position) {
  return array_remove_unstable((Array)arr, position);
}

// TODO:
/*
static inline index_s _prefix(_remove_range)
(_arr_type arr, index_s position, index_s count) {
  return array_remove_range((Array)arr, position, count);
}
//*/

// \brief Removes the last element from the array.
//
// \brief Note: the memory at the location of that element will remain unchanged
//    until another element is added in its place.
//
// \returns The size of the array after removing the element.
static inline index_s _prefix(_pop_back)
(_arr_type arr) {
  return array_pop_back((Array)arr);
}

// \brief Returns a copy of the element at the given position.
// \brief Will assert if the given index is out of bounds.
//
// \param index - the index at which to retrieve the item from
//
// \returns A copy of the indexed element.
static inline con_type _prefix(_get)
(const _arr_type arr, index_s index) {
  con_type* element = array_ref((Array)arr, index);
  assert(element != NULL);
  return *element;
}

// \brief Gets a copy of the first element of the array.
// \brief Will assert if called on an empty array.
//
// \returns A copy of the first element.
static inline con_type _prefix(_get_front)
(const _arr_type arr) {
  assert(arr->size > 0);
  return *arr->arr;
}

// \brief Returns a copy of the last element in the array.
// \brief Will assert if the array is empty.
//
// \returns A copy of the last element.
static inline con_type _prefix(_get_back)
(const _arr_type arr) {
  assert(arr->size > 0);
  return *(con_type*)array_ref_back((Array)arr);
}

// \brief Returns a reference to the element at the given position, or NULL if
//    the index is not valid. Note: an index that is within the capacity of the
//    array but outside the size will still return NULL.
//
// \param index - the index at which to retrieve the item from
//
// \returns A pointer to the indexed element.
static inline con_type* _prefix(_ref)
(_arr_type arr, index_s index) {
  return (con_type*)array_ref((Array)arr, index);
}

// \returns A pointer to the first element in the array, or NULL if empty.
static inline con_type* _prefix(_ref_front)
(_arr_type arr) {
  assert(arr);
  return arr->arr;
}

// \returns A pointer to the last element in the array, or NULL if empty.
static inline con_type* _prefix(_ref_back)
(_arr_type arr) {
  return (con_type*)array_ref_back((Array)arr);
}

// \brief Copies the value at the given index into the referenced output object.
// \brief If the index is invalid, no copy is performed.
//
// \param index - the index at which to retrieve the item from
//
// \param out_element - a pointer to the object to copy the data into
//
// \returns True if an element was written, false otherwise.
static inline bool _prefix(_read)
(const _arr_type arr, index_s index, con_type* out_element) {
  return array_read((Array)arr, index, out_element);
}

// \brief Writes a copy of the first element in the array into the memory
//    pointed to by out_element. No change is made if array is empty.
//
// \returns True if an element is written, false otherwise.
static inline bool _prefix(_read_front)
(const _arr_type arr, con_type* out_element) {
  assert(arr);
  assert(out_element);
  if (arr->size <= 0) return false;
  *out_element = *arr->arr;
  return true;
}

// \brief Writes a copy of the last element in the array into the memory
//    pointed to by out_element. No change is made if array is empty.
//
// \returns True if an element was written, false otherwise.
static inline bool _prefix(_read_back)
(const _arr_type arr, con_type* out_element) {
  assert(arr->size > 0);
  return array_read_back((Array)arr, out_element);
}

#undef _arr_type
#undef _full_prefix
#undef _prefix

#endif

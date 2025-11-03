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

#ifndef MCLIB_DYNAMIC_ARRAY_H_
#define MCLIB_DYNAMIC_ARRAY_H_

//
// Dynamic Array Container
//
// A dynamic array for generic data (void*) with the option of setup for
// type-specific variants. The array "owns" the data it contains, and allows
// access through spans, which remain valid until any function that modifies the
// size of the array is used.
//
// In order to define type-specific continers, include or re-include the header
// after #defining con_type with the desired type, and optionally con_prefix to
// set the prefix type specifier (if not set, the type will be used directly).
//
// When defined, the following will be created (inline, with no overhead):
//
// #define con_type V
// #define con_prefix v
// #define con_cmp compare_fn // optional, reuqired for sort/search functions
// #include "span.h" // prerequisite for array type
// #include "array.h"
// #undef con_type
// #undef con_prefix
// #undef con_cmp
//
// // Create, Setup, Delete
// Array_V    arr_v_new();
// Array_V    arr_v_new_reserve(index_t capacity);
// void       arr_v_reserve(Array_V, index_t capacity);
// void       arr_v_truncate(Array_V, index_t capacity);
// void       arr_v_clear(Array_V);
// void       arr_v_free(Array_V);
// void       arr_v_delete(Array_V*);
// T*         arr_v_release(Array_V*);
//
// // Item Addition
// void       arr_v_insert(Array_V, index_t position, V element);
// void       arr_v_insert_back(Array_V, V element);
// void       arr_v_insert_range(Array_V, index_t position, span_t range);
// void       arr_v_insert_back_range(Array_V, span_t range);
// void       arr_v_push_back(Array_V, V element);
// void       arr_v_write_back(Array_V, const V* element);
// bool       arr_v_set_back(Array_V, V element);
// V[1]       arr_v_emplace(Array_V, index_t position);
// V[1]       arr_v_emplace_back(Array_V);
// slice_V_t  arr_v_emplace_range(Array_V, index_t position, index_t count);
// slice_V_t  arr_v_emplace_back_range(Array_V, index_t count);
//
// // Item Removal
// index_t    arr_v_remove(Array_V, position);
// index_t    arr_v_remove_range(Array_V, position, count);
// index_t    arr_v_remove_unstable(Array_V, position);
// index_t    arr_v_pop_back(Array_V);
//
// // Accessors
// V          arr_v_get(Array_V, index_t index);
// V          arr_v_get_back(Array_V);
// void       arr_v_write(Array_V, index_t position, const V* element);
// bool       arr_v_set(Array_V, index_t index, V element);
// V*         arr_v_ref(Array_V, index_t index);
// V*         arr_v_ref_back(Array_V);
// bool       arr_v_read(Array_V, index_t index, V* out);
// bool       arr_v_read_back(Array_V, V* out);
//
// // Algorithm
// void       arr_v_reverse(Array_V);
// void       arr_v_filter(Array_V, predicate);
// void       arr_v_sort(Array_V, compare_fn cmp);
// V          arr_v_find(Array_V, predicate);
// V*         arr_v_ref_find(Array_V, predicate);
//

#include "types.h"
#include "span_base.h"

typedef struct array_t {
  union {
    span_t    const span;
    view_t    const view;
    struct {
      void*   const begin;
      void*   const end;
    };
  };
  index_t     const element_size;
  index_t     const capacity;
  index_t     const size;
  index_t     const size_bytes;
  //Allocator   const alloc;
} array_t, *Array;

#define   arr_new(TYPE) iarr_new(sizeof(TYPE))
#define   arr_new_reserve(TYPE, capacity) \
                          iarr_new_reserve(sizeof(TYPE), capacity)
#define   arr_build(TYPE) iarr_build(sizeof(TYPE))
#define   arr_build_reserve(TYPE, capacity) \
                          iarr_build_reserve(sizeof(TYPE), capacity)
Array    iarr_new(index_t elemenet_size);
Array    iarr_new_reserve(index_t element_size, index_t capacity);
array_t  iarr_build(index_t element_size);
array_t  iarr_build_reserve(index_t element_size, index_t capacity);
Array     arr_copy(Array to_copy);
Array     arr_copy_span(span_t to_copy, index_t element_size);
//Array   arr_copy_span_deep(span_t to_copy, void (*copy_fn)(void* dst, void* src));
void      arr_reserve(Array array, index_t capacity);
void      arr_truncate(Array array, index_t capacity);
void      arr_trim(Array array);
void      arr_clear(Array array);
void      arr_free(Array array);
void      arr_delete(Array* array);
span_t    arr_release(Array* array);
void*     arr_emplace(Array array, index_t position);
void*     arr_emplace_back(Array array);
span_t    arr_emplace_range(Array array, index_t position, index_t count);
span_t    arr_emplace_back_range(Array array, index_t count);
void      arr_insert(Array array, index_t position, const void* in_element);
void      arr_insert_back(Array array, const void* in_element);
void      arr_insert_range(Array array, index_t position, span_t range);
void      arr_insert_back_range(Array array, span_t range);
void      arr_write(Array array, index_t index, const void* in_element);
bool      arr_remove(Array array, index_t position);
bool      arr_remove_unstable(Array array, index_t position);
bool      arr_remove_range(Array array, index_t position, index_t count);
bool      arr_remove_range_unstable(Array array, index_t pos, index_t count);
bool      arr_pop_back(Array array);
bool      arr_pop_last(Array array, index_t count);
void*     arr_ref(Array array, index_t index);
void*     arr_ref_back(Array array);
bool      arr_read(const Array array, index_t index, void* out_element);
bool      arr_read_back(const Array array, void* out_element);
bool      arr_contains(const Array array, const void* to_find);
//void    arr_sort(Array array, bool (*cmp)(const void* lhs, const void* rhs));
//void    arr_ref_search
//void*   arr_ref_find(Array array, bool (*predicate)(const void* el));
//void    arr_filter(Array array, bool (*filter)(const void* el));

// \brief A macro shorthand to write foreach loops with any dynamic Array or
//    Array-based sub-types.
//
// \brief usage example:
// \brief MyType* arr_foreach(iterator, array) { use(iterator); }
#define arr_foreach(VAR, ARRAY)                                               \
  arr_foreach_index(VAR, MACRO_CONCAT(_arrit_, __LINE__), ARRAY)              //

// \brief A macro shorthand to write foreach loops with any dynamic Array or
//    Array-based sub-types. Includes a tracking index value as well.
//
// \brief note: using VAR + i*s instead of just ++VAR in order to ensure the
//    loop will continue to work in cases where a resize is performed during
//    iteration.
//
// \brief usage example:
// \brief MyType* arr_foreach_index(iter, i, array) { other[i] = *iter; }
#define arr_foreach_index(VAR, INDEX, ARRAY)                                  \
  VAR = (ARRAY)->begin;                                                       \
  assert(sizeof(*VAR) == ARRAY->element_size);                                \
  for (index_t INDEX = 0; INDEX < ARRAY->size; ++INDEX,                       \
    VAR = (void*)((byte*)ARRAY->begin + INDEX * sizeof(*VAR))                 \
  )                                                                           //

// TODO: would it be better to also track an offset rather than multiply?

// inlines for some alias functions

static inline void arr_write_back(Array array, const void* in_element) {
  arr_insert_back(array, in_element);
}

#endif

// specialized container/template type
#ifdef con_type

// The matching span type is required, but to avoid collisoins since we can't do
//    include guards for specializations, we have to require that the matching
//    span type was already created before this one.
// #include "span.h"

// Specialized container functions are declared as arr_<prefix>_<fn>
//    ex: - if con_prefix is 'str', you'll get a function arr_str_push_back
//        - if con_prefix is not set, you'll get arr_String_push_back

#ifdef con_prefix
# define _con_name con_prefix
#else
# define _con_name con_type
#endif

#define _arr_type MACRO_CONCAT(Array_, _con_name)
#define _arr_local_type MACRO_CONCAT3(array_, _con_name, _t)
#define _prefix(_FN) MACRO_CONCAT3(arr_, _con_name, _FN)
#define _span_type MACRO_CONCAT3(span_, _con_name, _t)
#define _span_prefix(_FN) MACRO_CONCAT3(span_, _con_name, _FN)

// Annoyingly have to redefine the struct to match - if we just typedef the
//    pointer type, it'll happily accept either as equivalent, but the whole
//    point is to prompt type errors.
typedef struct _arr_local_type {
  union {
    _span_type span;
    struct {
      union {
        con_type* const begin;
      };
      con_type* const end;
      index_t const element_size;
    };
  };
  index_t const capacity;
  index_t const size;
  index_t const size_bytes;
} _arr_local_type, *_arr_type;

// \brief Initializes a new array of the given type. Allocates no new space for
//    the array contents until an item is added.
//
// \returns A new empty dynamic array, ready for use.
static inline _arr_type _prefix(_new)
(void) {
  return (_arr_type) { (_arr_type)arr_new(con_type) };
}

// \brief Initialies a new array of the given type. Pre-allocates space for N
//    elements to be added without needing to expand the array. The array after
//    initialization is still empty.
//
// \param capacity - the number of elements to reserve space for
//
// \returns A new empty dynamic array with the given capacity.
static inline _arr_type _prefix(_new_reserve)
(index_t capacity) {
  return (_arr_type)arr_new_reserve(con_type, capacity);
}

// \brief Initializes an array of the given type in local storage. The contents
//    of the array will still be on the heap, but the header will be stored on
//    the stack.
//
// \brief When using these types, pass the `array_t` header to the `arr_`
//    functions using `&the_array`.
//
// \brief Note: when finished with the array, to deallocate pass its address to
//    `arr_free` rather than trying to pass it to `arr_delete`.
//
// \returns An empty dynamic array header that can be used with the array
//    functions by passing its address (ie, `&my_arr`).
static inline _arr_local_type _prefix(_build)
(void) {
  array_t ret = arr_build(con_type);
  return *(_arr_local_type*)(&ret);
}

// \brief Initializes an array of the given type in local storage. Pre-allocates
//    heap space for N elements to be added without needing to expand the array.
//
//
// \brief Note: when finished with the array, to deallocate pass its address to
//    `arr_free` rather than trying to pass it to `arr_delete`.
//
// \param capacity - the number of elements to reserve space for
//
// \returns An empty dynamic array header that can be used with the array
//    functions by passing its address (ie, `&my_arr`).
static inline _arr_local_type _prefix(_build_reserve)
(index_t capacity) {
  array_t ret = arr_build_reserve(con_type, capacity);
  return *(_arr_local_type*)(&ret);
}

// \brief Reserves space in the array so that it can contain at least N
//    elements. This will not reserve space for N _additional_ elements, any
//    items already in the array will still count towards the final capacity.
//
// \param capacity - the number of elements to reserve space for
static inline void _prefix(_reserve)
(_arr_type arr, index_t capacity) {
  arr_reserve((Array)arr, capacity);
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
(_arr_type arr, index_t capacity) {
  arr_truncate((Array)arr, capacity);
}

// \brief Performs a soft-delete of the array contents without changing
//    capacity or freeing any allocations.
static inline void _prefix(_clear)
(_arr_type arr) {
  arr_clear((Array)arr);
}

// \brief Frees the contents of the array back to the allocator. The array
//    object itself will still be stored in memory and can still be used.
static inline void _prefix(_free)
(_arr_type arr) {
  arr_free((Array)arr);
}

// \brief Deletes the array object and its contents from memory. Once deleted,
//    the provided pointer reference will be nulled.
static inline void _prefix(_delete)
(_arr_type* p_arr) {
  arr_delete((Array*)p_arr);
}

// \brief Deletes the array object without erasing the data.
//
// \returns The old contents of the array without freeing it.
static inline _span_type _prefix(_release)
(_arr_type* p_arr) {
  span_t span = arr_release((Array*)p_arr);
  return _span_prefix(_cast)(span);
}

// \brief Inserts a copy of the given element into the given position in the
//    array. Elements after the insert position will be moved one space forward.
//
// \param position - the index at which the new element will be accessed
//
// \param element - the element to insert into the array
static inline void _prefix(_add)
(_arr_type arr, index_t position, con_type element) {
  arr_insert((Array)arr, position, &element);
}

// \brief Inserts a copy of the given element into the back of the array.
//
// \param element - the element to insert into the array
static inline void _prefix(_add_back)
(_arr_type arr, con_type element) {
  arr_insert_back((Array)arr, &element);
}

// \brief Inserts a copy of the given element into the back of the array.
// \brief An extra alias for arr_X_add_back.
//
// \param element - the element to insert into the array
static inline void _prefix(_push_back)
(_arr_type arr, con_type element) {
  arr_insert_back((Array)arr, &element);
}

// \brief Inserts space for an element in the array and returns a pointer to it
//    without performing any initialization or copying into the given position.
//
// \param position - the index at which the new element will be accessed
//
// \returns A pointer to the newly added and uninitialized element.
static inline con_type* _prefix(_emplace)
(_arr_type arr, index_t position) {
  return arr_emplace((Array)arr, position);
}

// \brief Inserts space for an element at the back of the array and returns a
//    pointer to it without performing any initialization.
//
// \returns A pointer to the newly added and uninitialized element.
static inline con_type* _prefix(_emplace_back)
(_arr_type arr) {
  return arr_emplace_back((Array)arr);
}

// \brief Inserts space for count elements in the array and returns a span
//    representing the new region without performing any initialization or
//    copying into the range.
//
// \param position - the index at which the first new element will be accessed.
//
// \param count - the number of elements to make space for
//
// \returns A range containing new uninitialized space in the array.
static inline _span_type _prefix(_emplace_range)
(_arr_type arr, index_t position, index_t count) {
  span_t ret = arr_emplace_range((Array)arr, position, count);
  return *(_span_type*)&ret;
}

// \brief Inserts space for a number of elements at the back of the array and
//    returns a range representing the newly allocated region. No initialization
//    is performed on any of the new elements.
//
// \returns A range containing the newly added uninitialized elements.
static inline _span_type _prefix(_emplace_back_range)
(_arr_type arr, index_t count) {
  span_t ret = arr_emplace_back_range((Array)arr, count);
  return *(_span_type*)&ret;
}

static inline void _prefix(_insert)
(_arr_type arr, index_t position, const con_type* in_element) {
  arr_insert((Array)arr, position, in_element);
}

static inline void _prefix(_insert_back)
(_arr_type arr, const con_type* in_element) {
  arr_insert_back((Array)arr, in_element);
}

static inline void _prefix(_insert_range)
(_arr_type arr, index_t position, _span_type range) {
  arr_insert_range((Array)arr, position, range.base);
}

static inline void _prefix(_insert_back_range)
(_arr_type arr, _span_type range) {
  arr_insert_back_range((Array)arr, range.base);
}

// \brief Inserts a copy of the element referenced by the given pointer into the
//    array at the given position.
//
// \param position - the index at which the new element will be accessed
//
// \param element - a pointer to the element to write into the array
static inline void _prefix(_write)
(_arr_type arr, index_t position, const con_type* element) {
  arr_write((Array)arr, position, element);
}

// \brief Inserts a copy of the element referenced by the given pointer into the
//    back of the array.
//
// \param element - a pointer to the element to write into the array
static inline void _prefix(_write_back)
(_arr_type arr, const con_type* element) {
  arr_write_back((Array)arr, element);
}

// \brief Removes the given element in the array, shifting the remaining items
//    to fill the space
//
// \param position - The index to remove
//
// \returns True if an item was removed, false if it was empty.
static inline bool _prefix(_remove)
(_arr_type arr, index_t position) {
  return arr_remove((Array)arr, position);
}

// \brief Removes a number of elements from a given position in the array,
//    shifting the remaining items to fill the space.
//
// \param position - The index of the first item to remove
//
// \param count - the number of items to remove
//
// \returns True if items were removed, false if the array was empty.
static inline bool _prefix(_remove_range)
(_arr_type arr, index_t position, index_t count) {
  return arr_remove_range((Array)arr, position, count);
}

// \brief Removes a number of elements from a given position in the array. The
//    space is filled by copying up to an equal number of items from the end of
//    the array rather than copying the entire contents of the remainder.
//
// \param position - The index of the first item to remove
//
// \param count - the number of items to remove
//
// \returns True if items were removed, false if the array was empty.
static inline bool _prefix(_remove_range_unstable)
(_arr_type arr, index_t position, index_t count) {
  return arr_remove_range_unstable((Array)arr, position, count);
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
static inline bool _prefix(_remove_unstable)
(_arr_type arr, index_t position) {
  return arr_remove_unstable((Array)arr, position);
}

// \brief Removes the last element from the array.
//
// \brief Note: the memory at the location of that element will remain unchanged
//    until another element is added in its place.
//
// \returns The size of the array after removing the element.
static inline bool _prefix(_pop_back)
(_arr_type arr) {
  return arr_pop_back((Array)arr);
}

// \brief Removes the last `count` elements from the array.
//
// \returns True if the size was reduced, false if it was empty.
static inline bool _prefix(_pop_last)
(_arr_type arr, index_t count) {
  return arr_pop_last((Array)arr, count);
}

// \brief Returns a copy of the element at the given position.
// \brief Will assert if the given index is out of bounds.
//
// \param index - the index at which to retrieve the item from
//
// \returns A copy of the indexed element.
static inline con_type _prefix(_get)
(const _arr_type arr, index_t index) {
  assert(arr);
  con_type* element = _span_prefix(_ref)(arr->span, index);
  assert(element);
  return *element;
}

// \brief Returns a copy of the last element in the array.
// \brief Will assert if the array is empty.
//
// \returns A copy of the last element.
static inline con_type _prefix(_get_back)
(const _arr_type arr) {
  assert(arr);
  con_type* element = _span_prefix(_ref_back)(arr->span);
  assert(element);
  return *element;
}

// \brief Returns a reference to the element at the given position, or NULL if
//    the index is not valid. Note: an index that is within the capacity of the
//    array but outside the size will still return NULL.
//
// \param index - the index at which to retrieve the item from
//
// \returns A pointer to the indexed element.
static inline con_type* _prefix(_ref)
(_arr_type arr, index_t index) {
  assert(arr);
  return _span_prefix(_ref)(arr->span, index);
}

// \returns A pointer to the last element in the array, or NULL if empty.
static inline con_type* _prefix(_ref_back)
(_arr_type arr) {
  assert(arr);
  return _span_prefix(_ref_back)(arr->span);
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
(const _arr_type arr, index_t index, con_type* out_element) {
  assert(arr);
  assert(out_element);
  const con_type* ptr = _span_prefix(_ref)(arr->span, index);
  if (!ptr) return false;
  *out_element = *ptr;
  return true;
}

// \brief Writes a copy of the last element in the array into the memory
//    pointed to by out_element. No change is made if array is empty.
//
// \returns True if an element was written, false otherwise.
static inline bool _prefix(_read_back)
(const _arr_type arr, con_type* out_element) {
  assert(arr);
  assert(out_element);
  if (arr->size <= 0) return false;
  *out_element = *(arr->end - 1);
  return true;
}

// Algorithm

// \brief Reverses the order of the array
static inline void _prefix(_reverse)
(_arr_type arr) {
  assert(arr);
  assert(arr->size >= 0);
  _span_prefix(_reverse)(arr->span);
}

// \brief Linearly searches the array for an element that is an exact binary
//    match for to_find.
//
// \returns True if a match was found, false otherwise
static inline bool _prefix(_contains)
(const _arr_type arr, con_type to_find) {
  return arr_contains((Array)arr, &to_find);
}

#ifdef con_cmp

// \brief Sorts the array in place
//
//
static inline void _prefix(_sort)
(_arr_type arr) {
  arr_sort((Array)arr, con_cmp);
}

static inline void _prefix(_shuffle)
(_arr_type arr) {
  _span_prefix(_shuffle)(arr->span, arr->element_size);
}

static inline index_t _prefix(_find)
(_arr_type arr, con_type to_find) {
  return arr_find((Array)arr, to_find, con_cmp);
}

#endif // con_cmp

#undef _con_name
#undef _arr_type
#undef _arr_local_type
#undef _prefix
#undef _span_type
#undef _span_prefix

#endif

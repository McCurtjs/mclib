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

#include "array.h"

#define CSPEC_CUSTOM_TYPES \
  Array: "Array"

#include "cspec.h"

describe(array_construction) {

  it("creates an empty array") {
    Array arr = array_new(int);
    expect(arr to not be_null);
    expect(arr->begin to be_null);
    expect(arr->size to be_zero);
    expect(arr->capacity to be_zero);
    array_delete(&arr);
  }

  it("creates an array with a specific reserve capacity") {
    Array arr = array_new_reserve(double, 10);
    expect(arr to not be_null);
    expect(arr->begin to not be_null);
    expect(arr->size to be_zero);
    expect(arr->capacity, == , 10);
    array_delete(&arr);
  }
  /*
  it("copies data from a span") {
    int data[] = {1, 2, 3};
    span_t span = SPAN(data);
    Array arr = array_copy_span(span);
    expect(arr->size, == , 3);
    expect(((int*)arr->begin)[2], == , 3);
    array_delete(&arr);
  }
  //*/
}

describe(array_push_and_pop) {

  it("pushes and pops elements at the back") {
    Array arr = array_new(int);
    expect(arr);

    int value = 42;
    expect(array_write_back to be_one given(arr, &value));
    expect(arr->size to be_one);

    int out = 0;
    expect(array_read to be_true given(arr, 0, &out));
    expect(out, == , value);

    expect(array_read to be_false given(arr, 1, &out));

    out = 0;
    expect(array_read_back to be_true given(arr, &out));
    expect(out, == , value);

    int* outp = NULL;
    outp = array_ref(arr, 0);
    expect(outp to not be_null);
    expect(*outp, == , value);

    expect(array_ref to be_null given(arr, 1));

    outp = NULL;
    outp = array_ref_back(arr);
    expect(outp to not be_null);
    expect(*outp, == , value);

    expect(array_pop_back to be_zero given(arr));
    expect(arr->size to be_zero);
    expect(arr->capacity to not be_zero);
    expect(arr->begin to not be_zero);

    expect(array_read_back to be_false given(arr, &out));
    expect(array_ref_back to be_null given(arr));

    array_delete(&arr);
    expect(arr to be_null);
  }

  it("grows capacity when pushing beyond current limit") {
    Array arr = array_new_reserve(int, 1);
    int vals[] = {1, 2, 3};
    for (int i = 0; i < 3; ++i) array_write_back(arr, &vals[i]);
    expect(arr->capacity, >= , 3);
    array_delete(&arr);
  }

}

describe(on_stack_construction_and_use) {

  it("creates an empty array on the stack") {
    array_t arr = arr_build(int);
    expect(arr.begin to be_null);
    expect(arr.size to be_zero);
    expect(arr.capacity to be_zero);
    // no need to deallocate, nothing was allocated
  }

}

describe(array_insert_and_remove) {

  it("inserts elements at a specific index") {
    Array arr = array_new(int);
    int vals[] = {1, 2, 3};
    for (int i = 0; i < 3; ++i) array_write_back(arr, &vals[i]);
    int* value = array_emplace(arr, 1);
    *value = 99;
    expect(((int*)arr->begin)[1], == , 99);
    expect(arr->size, == , 4);
    array_delete(&arr);
  }

  it("removes elements by index range") {
    Array arr = array_new(int);
    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i) array_write_back(arr, &vals[i]);
    array_remove_range(arr, 1, 2); // remove [2,3]
    expect(arr->size, == , 3);
    expect(((int*)arr->begin)[1], == , 4);
    array_delete(&arr);
  }

  it("supports unstable removal") {
    Array arr = array_new(int);
    int vals[] = {1, 2, 3, 4};
    for (int i = 0; i < 4; ++i) array_write_back(arr, &vals[i]);
    array_remove_unstable(arr, 1);
    expect(arr->size, == , 3);
    array_delete(&arr);
  }

}

describe(array_resize_and_clear) {

  it("can resize up or down") {
    Array arr = array_new(int);
    array_emplace_back_range(arr, 10);
    expect(arr->size, == , 10);
    array_truncate(arr, 3);
    expect(arr->size, == , 3);
    array_delete(&arr);
  }

  it("clears all elements") {
    Array arr = array_new(int);
    int v = 5;
    array_write_back(arr, &v);
    array_clear(arr);
    expect(arr->size, == , 0);
    array_delete(&arr);
  }

}

describe(array_iteration) {

  it("iterates using the foreach macro") {
    Array arr = array_new(int);
    for (int i = 0; i < 5; ++i) array_write_back(arr, &i);

    int sum = 0;
    int* array_foreach(value, arr) sum += *value;

    expect(sum, == , 10);
    array_delete(&arr);
  }

}

describe(array_bounds_and_errors) {

  it("handles out-of-bounds index gracefully") {
    Array arr = array_new(int);
    expect(array_ref(arr, 0) == NULL);
    array_delete(&arr);
  }

  it("is safe to delete NULL arrays") {
    Array arr = NULL;
    array_delete(&arr);
    expect(arr == NULL);
  }

}

test_suite(tests_array) {
  test_group(array_construction),
  test_group(array_push_and_pop),
  test_group(array_insert_and_remove),
  test_group(array_resize_and_clear),
  test_group(array_iteration),
  test_group(array_bounds_and_errors),
  test_suite_end
};

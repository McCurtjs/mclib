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

describe(arr_construction) {

  it("creates an empty array") {
    Array arr = arr_new(int);
    expect(arr to not be_null);
    expect(arr->begin to be_null);
    expect(arr->size to be_zero);
    expect(arr->capacity to be_zero);
    arr_delete(&arr);
  }

  it("creates an array with a specific reserve capacity") {
    Array arr = arr_new_reserve(double, 10);
    expect(arr to not be_null);
    expect(arr->begin to not be_null);
    expect(arr->size to be_zero);
    expect(arr->capacity, == , 10);
    arr_delete(&arr);
  }
  /*
  it("copies data from a span") {
    int data[] = {1, 2, 3};
    span_t span = SPAN(data);
    Array arr = arr_copy_span(span);
    expect(arr->size, == , 3);
    expect(((int*)arr->begin)[2], == , 3);
    arr_delete(&arr);
  }
  //*/
}

describe(arr_push_and_pop) {
  int test = 42;

  Array arr = arr_new(int);
  expect(arr);
  expect(arr->size to be_zero);
  expect(arr->capacity to be_zero);

  it("emplaces a value to the back") {
    int* value = arr_emplace_back(arr);
    expect(value to not be_null);
    *value = test;
    expect(*((int*)arr->begin), == , test);
  }

  it("writes a value from an argument to the back of the array") {
    arr_insert_back(arr, &test);
    expect(arr->size to be_one);
    expect(arr->capacity to not be_zero);
  }

  it("writes more values to force a resize") {
    for (int i = 0; i < 10; ++i) {
      arr_write_back(arr, &i);
    }

    expect(arr->size, == , 10);
    expect(arr->capacity, >= , 10);

    int* array = arr->begin;
    for (int i = 0; i < 10; ++i) {
      expect(array[i], == , i);
    }

    arr_clear(arr);
    arr_emplace_back(arr);
  }

  after {
    expect(arr->size to be_one);
    expect(arr_pop_back to be_true given(arr));
    expect(arr->size to be_zero);
    arr_delete(&arr);
  }

}

describe(arr_deleting) {

  Array arr = arr_new(int);
  expect(arr to not be_null);

  it("deletes the array normally") {
    arr_delete(&arr);
    expect(arr to be_null);
  }

  it("frees the contents of the array, but doesn't delete it") {
    arr_emplace_back(arr);
    expect(arr->size to be_one);
    expect(arr->begin to not be_null);
    expect(arr->capacity to not be_zero);

    arr_free(arr);
    expect(arr != NULL);
    expect(arr->size to be_zero);
    expect(arr->begin to be_null);
    expect(arr->capacity to be_zero);

    arr_delete(&arr);
  }

  it("empties the array without freeing the memory") {
    arr_emplace_back(arr);
    expect(arr->size to be_one);
    expect(arr->begin to not be_null);
    expect(arr->capacity to not be_zero);

    arr_clear(arr);
    expect(arr->size to be_zero);
    expect(arr->begin to not be_null);
    expect(arr->capacity to not be_zero);

    arr_delete(&arr);
  }

  it("deletes the array header, but leaves the memory intact") {
    arr_emplace_back(arr);
    int* arr_begin = arr->begin;
    expect(arr->size to be_one);
    expect(arr_begin to not be_null);
    expect(arr->capacity to not be_zero);

    span_t data = arr_release(&arr);
    expect(arr to be_null);
    expect(data.begin, == ,arr_begin);

    free(data.begin);
  }

}

describe(arr_capacity) {

  Array arr = NULL;

  context("reserving the capacity at object creation") {

    it("can allocate an array with a preset capacity") {
      arr = arr_new_reserve(int, 10);
      expect(arr->size to be_zero);
      expect(arr->capacity, == , 10);
    }

    it("can allocate an array with a different type size") {
      arr = arr_new_reserve(double, 10);
      expect(arr->size to be_zero);
      expect(arr->capacity, == , 10);
    }

  }

  context("modifying the capacity after object creation") {

    arr = arr_new(int);
    expect(arr->capacity to be_zero);
    expect(arr->begin to be_null);

    it("increases the capacity without adjusting the size") {
      arr_reserve(arr, 10);
      expect(arr->size to be_zero);
      expect(arr->capacity, == , 10);
      expect(arr->begin to not be_null);
    }

    context("reducing the capacity") {

      arr_reserve(arr, 10);
      arr_emplace_back(arr);
      arr_emplace_back(arr);
      expect(arr->begin to not be_null);
      expect(arr->size, == , 2);
      expect(arr->capacity, == , 10);

      it("trims unused allocated space from the end of the array") {
        arr_trim(arr);
        expect(arr->size, == , 2);
        expect(arr->capacity, == , 2);
      }

      it("truncates the unused space to a given amount") {
        arr_truncate(arr, 5);
        expect(arr->size, == , 2);
        expect(arr->capacity, == , 5);
      }

      it("can truncate to the point of dropping elements") {
        arr_truncate(arr, 1);
        expect(arr->size, == , 1);
        expect(arr->capacity, == , 1);
      }

      after {
        expect(arr->begin to not be_null);
      }

      it("will free the memory when truncating to zero") {
        arr_truncate(arr, 0);
        expect(arr->size to be_zero);
        expect(arr->capacity to be_zero);
        expect(arr->begin to be_null);
      }

    }

  }

  after {
    arr_delete(&arr);
  }

}
/*
describe(array_accessors) {

  Array subject = arr_new(int);


  it("") {

    int out = 0;
    expect(arr_read to be_true given(arr, 0, &out));
    expect(out, == , value);

    expect(arr_read to be_false given(arr, 1, &out));

    out = 0;
    expect(arr_read_back to be_true given(arr, &out));
    expect(out, == , value);

    int* outp = NULL;
    outp = arr_ref(arr, 0);
    expect(outp to not be_null);
    expect(*outp, == , value);

    expect(arr_ref to be_null given(arr, 1));

    outp = NULL;
    outp = arr_ref_back(arr);
    expect(outp to not be_null);
    expect(*outp, == , value);

    expect(arr_pop_back to be_zero given(arr));
    expect(arr->size to be_zero);
    expect(arr->capacity to not be_zero);
    expect(arr->begin to not be_zero);

    expect(arr_read_back to be_false given(arr, &out));
    expect(arr_ref_back to be_null given(arr));

    arr_delete(&arr);
    expect(arr to be_null);
  }

  it("grows capacity when pushing beyond current limit") {
    Array arr = arr_new_reserve(int, 1);
    int vals[] = {1, 2, 3};
    for (int i = 0; i < 3; ++i) arr_write_back(arr, &vals[i]);
    expect(arr->capacity, >= , 3);
    arr_delete(&arr);
  }

}
*/

describe(arr_inserts) {
  Array arr = arr_new(int);
  for (int i = 0; i < 3; ++i) arr_write_back(arr, &i);
  expect(arr->size, == , 3);
  index_t expected_count = 4;
  int number = 42;
  int* expected = NULL;
  int* value = NULL;

  context("when inserting single items") {

    context("using emplace") {

      it("emplaces to the back") {
        expected = (int[]){ 0, 1, 2, 99 };
        value = arr_emplace_back(arr);
      }

      it("emplaces at exactly the end using an index") {
        expected = (int[]){ 0, 1, 2, 99 };
        value = arr_emplace(arr, arr->size);
      }

      it("emplaces at the beginning") {
        expected = (int[]){ 99, 0, 1, 2 };
        value = arr_emplace(arr, 0);
      }

      it("emplaces elements at a specific index") {
        expected = (int[]){ 0, 99, 1, 2 };
        value = arr_emplace(arr, 1);
      }

      it("fails when the value is past the end") {
        expect(to_assert);
        arr_emplace(arr, 50);
      }

      it("fails when the given index is negative") {
        expected = (int[]){ 0, 1, 99, 2 };
        value = arr_emplace(arr, -1);
      }

    }

    context("using insert") {

      value = &number;

      it("inserts a value to the end") {
        expected = (int[]){ 0, 1, 2, 42 };
        arr_insert_back(arr, &number);
      }

      it("inserts elements at a specific index") {
        expected = (int[]){ 0, 1, 42, 2 };
        arr_insert(arr, 2, &number);
      }

      it("fails when given a NULL value to copy from") {
        expect(to_assert);
        arr_insert(arr, 0, NULL);
      }

      it("fails to insert when the value is past the end") {
        expect(to_assert);
        arr_insert(arr, 50, &number);
      }

    }

    context("using write") {

      value = &number;

      it("writes a value to the end") {
        expected = (int[]){ 0, 1, 2, 42 };
        arr_write_back(arr, &number);
      }

      it("writes a value to arr->size") {
        expected = (int[]){ 0, 1, 2, 42 };
        arr_write(arr, arr->size, &number);
      }

      it("fails when given a NULL value to copy from") {
        expect(to_assert);
        arr_write(arr, 0, NULL);
      }

      it("fails to write when the value is past the end") {
        expect(to_assert);
        arr_write(arr, 50, &number);
      }

    }

    after {
      *value = 99;
    }

  }

  context("when emplacing as a range") {

    expected_count = 6;
    span_t result = span_empty;

    it("inserts at exactly the end") {
      expected = (int[]) { 0, 1, 2, 75, 76, 77 };
      result = arr_emplace_back_range(arr, 3);
    }

    it("inserts at exactly the end using an index") {
      expected = (int[]) { 0, 1, 2, 75, 76, 77 };
      result = arr_emplace_range(arr, arr->size, 3);
    }

    it("inserts at the beginning") {
      expected = (int[]) { 75, 76, 77, 0, 1, 2 };
      result = arr_emplace_range(arr, 0, 3);
    }

    it("inserts elements at a specific index") {
      expected = (int[]) { 0, 75, 76, 77, 1, 2 };
      result = arr_emplace_range(arr, 1, 3);
    }

    it("inserts elements at a specific index") {
      expected = (int[]){ 0, 75, 76, 77, 1, 2 };
      result = arr_emplace_range(arr, 1, 3);
      //arr_write_range()
    }

    it("inserts at the end when a large number is given") {
      expected = (int[]) { 0, 1, 2, 75, 76, 77 };
      result = arr_emplace_range(arr, arr->size, 3);
    }

    it("fails when the given index is negative") {
      expect(to_assert);
      arr_emplace_range(arr, -1, 3);
    }

    after {
      expect(result.end, == , ((int*)result.begin) + 3);
      span_foreach_index(value, i, result) {
        *value = (int)(i + 75);
      }
    }

  }

  after {
    expect(arr->size, == , expected_count);
    arr_foreach_index(value, i, arr) {
      expect(*value, == , expected[i]);
    }
    arr_delete(&arr);
  }

}

describe(arr_removal) {

  it("removes an element from the middle of the array") {
    Array arr = arr_new(int);
    int vals[] = { 1, 2, 3, 4, 5, 6 };
    for (int i = 0; i < 6; ++i) arr_write_back(arr, &vals[i]);
    arr_remove(arr, 2);
    expect(arr->size, == , 5);
    expect(arr->size_bytes, == , (index_t)(5 * sizeof(int)));
    arr_delete(&arr);
  }

  it("removes elements by index range") {
    Array arr = arr_new(int);
    int vals[] = { 1, 2, 3, 4, 5 };
    for (int i = 0; i < 5; ++i) arr_write_back(arr, &vals[i]);
    arr_remove_range(arr, 1, 2); // remove [2,3]
    expect(arr->size, == , 3);
    expect(((int*)arr->begin)[1], == , 4);
    arr_delete(&arr);
  }

  it("supports unstable removal") {
    Array arr = arr_new(int);
    int vals[] = { 1, 2, 3, 4 };
    for (int i = 0; i < 4; ++i) arr_write_back(arr, &vals[i]);
    arr_remove_unstable(arr, 1);
    expect(arr->size, == , 3);
    arr_delete(&arr);
  }

}

describe(arr_resize_and_clear) {

  it("can resize up or down") {
    Array arr = arr_new(int);
    arr_emplace_back_range(arr, 10);
    expect(arr->size, == , 10);
    arr_truncate(arr, 3);
    expect(arr->size, == , 3);
    arr_delete(&arr);
  }

  it("clears all elements") {
    Array arr = arr_new(int);
    int v = 5;
    arr_write_back(arr, &v);
    arr_clear(arr);
    expect(arr->size, == , 0);
    arr_delete(&arr);
  }

}

describe(arr_iteration) {

  it("iterates using the foreach macro") {
    Array arr = arr_new(int);
    for (int i = 0; i < 5; ++i) arr_write_back(arr, &i);

    int sum = 0;
    int* arr_foreach(value, arr) sum += *value;

    expect(sum, == , 10);
    arr_delete(&arr);
  }

}

describe(arr_bounds_and_errors) {

  it("handles out-of-bounds index gracefully") {
    Array arr = arr_new(int);
    expect(arr_ref(arr, 0) == NULL);
    arr_delete(&arr);
  }

  it("is safe to delete NULL arrays") {
    Array arr = NULL;
    arr_delete(&arr);
    expect(arr == NULL);
  }

}

#define con_type int
#include "view.h"
#undef con_type

describe(arr_on_stack_construction_and_use) {

  it("creates an empty array on the stack") {
    array_t arr = arr_build(int);
    expect(arr.begin to be_null);
    expect(arr.size to be_zero);
    expect(arr.capacity to be_zero);
    // no need to deallocate, nothing was allocated
    pair_view_t views = view_split_at((view_t) { 0 }, 0, 1);
    pair_view_int_t ints = view_int_split_at((view_int_t) { 0 }, 0);
    partition_view_int_t asdf = view_int_partition((view_int_t) { 0 }, NULL);
    UNUSED(views);
    UNUSED(ints);
    UNUSED(asdf);
  }

  it("can allocate space for the data segment") {
    array_t arr = arr_build(int);
    int* value = arr_emplace_back(&arr);
    *value = 99;
    int* val_2 = arr_ref_back(&arr);
    expect(*val_2, == , *value);
    arr_free(&arr);
    expect(arr.begin to be_null);
  }

}

test_suite(tests_array) {
  test_group(arr_construction),
  test_group(arr_capacity),
  test_group(arr_deleting),
  test_group(arr_push_and_pop),
  test_group(arr_inserts),
  test_group(arr_removal),
  test_group(arr_resize_and_clear),
  test_group(arr_iteration),
  test_group(arr_bounds_and_errors),
  test_group(arr_on_stack_construction_and_use),
  test_suite_end
};

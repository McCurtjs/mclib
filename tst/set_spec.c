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

#include "set.h"

#include "slice.h"

#include "str.h"

#define con_type slice_t
#define con_prefix str
#define con_type_hash_compare slice_hash_vptr, slice_compare_vptr
#define con_type_copy_delete slice_copy_vptr, slice_delete_vptr
#include "set.h"
#undef con_type_copy_delete
#undef con_type_hash_compare
#undef con_prefix
#undef con_type

#define CSPEC_CUSTOM_TYPES  \
  HSet: "HSet", HSet_str: "HSet_str", slice_t: "slice_t", slice_t*: "slice_t*"

#include "cspec.h"

describe(set_construction) {

  it("Takes no action when trying to delete a null pointer") {
    HSet set = NULL;
    set_delete(&set);
    set_delete(NULL);
    expect(malloc_count == 0);
    expect(free_count == 0);
  }

  it("Throws an assert when out of memory") {
    expect(malloc_to_fail);
    expect(to_assert);
    set_new(int, NULL, NULL);
  }

  it("Creates and deletes a map without any inserts or reads") {
    HSet set = set_new(int, NULL, NULL);
    expect(set to not be_null);
    expect(set->size to be_zero);
    expect(set->element_size == sizeof(int));
    set_delete(&set);
    expect(set to be_null);
  }

}

describe(set_slices) {

  HSet set = set_new(slice_t, slice_hash_vptr, slice_compare_vptr);

  it("can use slices for indexing") {
    set_insert(set, &S("test slice"));
    expect(set->size, == , 1);
    expect(set_contains to be_true given(set, &S("test slice")));

    slice_t first = *(slice_t*)set_ref(set, &S("test slice"));
    expect(slice_eq to be_true given(first, S("test slice")));

    const char data[] = "test slice xyz";
    slice_t second = slice_build(data, 10);
    expect(slice_eq to be_true given(second, S("test slice")));
    expect(first.begin, != , second.begin);

    // set can verify equality using the provided comparison functions despite
    //    the literal values being different
    expect(set_contains to be_true given(set, &second));

    // writing an existing value to a set is allowed, but only if the values are
    //    semantically similar. this is not true for insert.
    expect(set_write to be_false given(set, &second));

    expect(first.begin, != , data);
    first = *(slice_t*)set_ref(set, &S("test slice"));
    expect(first.begin, == , data);
  }

  after {
    set_delete(&set);
  }

}

/*

describe(set_string_keys) {

  HSet set = set_new(slice_t, slice_hash_vptr, slice_compare_vptr);

  it("can use slices to index") {
    int* value = set_emplace(set, &S("FIRST"));
    expect(value to not be_null);
    *value = 12;

    int* retrieved = set_ref(set, &S("FIRST"));
    expect(value to equal(retrieved));
    expect(*value to equal(*retrieved));

    retrieved = set_ref(set, &S("first"));
    expect(retrieved to be_null);
  }

  after{
    set_delete(&set);
  }

}
//*/

describe(set_ref) {

  HSet set = set_new(int, NULL, NULL);

  it("asserts if a NULL value is given for the key") {
    expect(to_assert);
    set_ref(set, NULL);
  }

  it("returns false if the key is not found in the map") {
    const int* ptr = set_ref(set, &(int){ 5 });
    expect(ptr to be_null);
  }

  after{
    set_delete(&set);
  }

}

describe(set_ensure) {

  HSet set = set_new(int, NULL, NULL);

  context("Critical failure cases") {

    it("throws an assert when given a NULL value") {
      expect(to_assert);
      set_ensure(set, NULL);
    }

    it("throws an assert when given a NULL set") {
      expect(to_assert);
      set_ensure(NULL, &(int){ 1 });
    }

  }

  it("will initialize a default capacity o 8 when adding to an empty set") {
    expect(set->size to be_zero);
    expect(set->capacity to be_zero);
    set_ensure(set, &(int){ 5 });
    expect(set->size to be_one);
    expect(set->capacity, == , 8);
  }

  it("adds an element to the map and reads it back") {
    set_ensure_t result = set_ensure(set, &(int){ 5 });

    expect(result.is_new to be_true);
    expect(*(int*)result.value to equal(5));
    expect(set->size to equal(1));

    result = set_ensure(set, &(int){ 0xBADF00D5 });

    expect(result.is_new to be_true);
    expect(*(int*)result.value to equal((int)0xBADF00D5));
    expect(set->size to equal(2));

    expect(set_contains to be_true given(set, &(int){ 5 }));
    expect(set_contains to be_true given(set, &(int){ 0xBADF00D5 }));
  }

  it("grows the map when it reaches 75% capacity") {
    int i;
    for (i = 1; i < 6; ++i) {
      set_ensure(set, &i);
      expect(set->size, == , i);
    }

    // validate reference values
    for (i = 1; i < 6; ++i) {
      const int* result = set_ref(set, &i);
      expect(*result to equal(i));
    }

    expect(set->capacity, == , 8);
    set_ensure(set, &i);
    expect(set->capacity, == , 16);

    // validate values again to make sure they moved
    for (i = 1; i < 7; ++i) {
      const int* result = set_ref(set, &i);
      expect(*result to equal(i));
    }
  }

  it("returns the existing element when a duplicate key is used") {
    int value = 1337;
    set_ensure(set, &value);
    expect(set->size to be_one);

    set_ensure_t find = set_ensure(set, &value);

    expect(find.is_new to be_false);
    expect(*(int*)find.value to equal(value));
  }

  after {
    set_delete(&set);
  }

}

describe(set_insert) {

  HSet set = set_new(int, NULL, NULL);
  int value = 12;

  context("critical failure cases") {

    it("will fail when given a null value") {
      expect(to_assert);
      set_insert(set, NULL);
    }

    it("will fail when given a null set") {
      expect(to_assert);
      set_insert(NULL, &value);
    }

  }

  it("can insert a value") {
    expect(set_insert to be_true given(set, &value));
    expect(set_contains to be_true given(set, &value));
  }

  it("can insert multiple non-conflicting values") {
    expect(set_insert to be_true given(set, &(int){ 5 }));
    expect(set_insert to be_true given(set, &(int){ 3 }));
    expect(set->size, == , 2);
  }

  it("will not insert conflicting values") {
    expect(set_insert to be_true given(set, &value));
    expect(set_insert to be_false given(set, &value));
    expect(set->size, == , 1);
  }

  after {
    set_delete(&set);
  }

}

describe(set_write) {

  HSet set = set_new(int, NULL, NULL);
  int value = 12;

  context("critical failure cases") {

    it("will fail when given a null value") {
      expect(to_assert);
      set_write(set, NULL);
    }

    it("will fail when given a null set") {
      expect(to_assert);
      set_write(NULL, &value);
    }

  }

  it("can write a new value to the set") {
    expect(set_write to be_true given(set, &value));
    expect(set_contains to be_true given(set, &value));
  }

  it("can write multiple non-conflicting values") {
    expect(set_write to be_true given(set, &(int){ 12 }));
    expect(set_write to be_true given(set, &(int){ 56 }));
    expect(set->size, == , 2);
  }

  it("will overwrite but return false given a duplicate value") {
    expect(set_write to be_true given(set, &value));
    expect(set_write to be_false given(set, &value));
    expect(set->size, == , 1);
  }

  after {
    set_delete(&set);
  }

}

describe(set_next) {

  HSet set = set_new(int, NULL, NULL);

  it("can iterate an empty set") {
    int acc = 0;
    const int* set_foreach(i, set) acc += *i;
    expect(acc to be_zero);
  }

  context("for a set containing ten items") {

    for (int i = 0; i < 10; ++i) {
      set_insert(set, &i);
    }

    it("can iterate through the set and get each value") {
      // the values in a hash set are not guaranteed to be in order, so
      //    validate presence using an accumulator
      int acc = 0;
      const int* set_foreach(i, set) acc += *i;
      expect(acc, == , 45);
    }

  }

  after {
    set_delete(&set);
  }

}

describe(set_remove) {

  HSet set = set_new(int, NULL, NULL);

  it("asserts if a NULL value is given") {
    expect(to_assert);
    set_remove(set, NULL);
  }

  it("returns false when removing a value that isn't in the set") {
    expect(set_remove to be_false given(set, &(int){ 10 }));
  }

  it("can remove a value from the set") {
    set_insert(set, &(int){ 5 });
    expect(set->size to be_one);
    expect(set_remove to be_true given(set, &(int){ 5 }));
    expect(set->size to be_zero);
  }

  context("when values are stacked into a bucket") {

    int value_2 = 2; // 2 and 4 have conflicting hashes for an 8 slot container
    int value_4 = 4;
    set_insert(set, &value_2);
    set_insert(set, &value_4);
    expect(set->size to equal(2));

    it("will remove values from the root of the bucket and leave the rest") {
      expect(set_remove to be_true given(set, &value_2));
      const int* value = set_ref(set, &value_4);
      expect(*value to equal(value_4));
      expect(set_ref to be_null given(set, &value_2));
    }

    it("will remove values later in the bucket without affecting the root") {
      expect(set_remove to be_true given(set, &value_4));
      const int* value = set_ref(set, &value_2);
      expect(value to not be_null);
      expect(*value to equal(value_2));
      expect(set_contains to be_false given(set, &value_4));
    }

  }

  after {
    set_delete(&set);
  }

}

// Defining element and key types

describe(set_specialized) {

  HSet_str strs = set_str_new();

  slice_t to_insert = S("some value");

  it("just works") {
    set_str_ensure_t result = set_str_ensure(strs, &to_insert);
    expect(result.is_new to be_true);
    expect(result.value->begin, != , to_insert.begin);
    expect(strs->size, == , 1);
    expect(strs->capacity, == , 8);

    to_insert = S("another string");
    expect(set_str_insert to be_true given(strs, &to_insert));
    expect(strs->size, == , 2);
    expect(set_str_ref(strs, &to_insert)->begin, != , to_insert.begin);

    to_insert = S("third string");
    expect(set_str_add to be_true given(strs, to_insert));
    expect(strs->size, == , 3);
    expect(set_str_get(strs, to_insert).begin, != , to_insert.begin);

    expect(set_str_erase to be_false given(strs, S("invisible")));
    expect(strs->size, == , 3);
    expect(set_str_erase to be_true given(strs, S("another string")));
    expect(strs->size, == , 2);
  }

  after {
    set_str_delete(&strs);
  }

}

test_suite(tests_set) {
  test_group(set_construction),
  test_group(set_slices),
  test_group(set_ref),
  test_group(set_ensure),
  test_group(set_insert),
  test_group(set_write),
  test_group(set_next),
  test_group(set_remove),
  test_group(set_specialized),
  test_suite_end
};

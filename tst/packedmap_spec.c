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

#include "packedmap.h"

#define CSPEC_CUSTOM_TYPES \
  PackedMap: "PackedMap", slotkey_t: "slotkey_t", view_t: "view_t"

#include "cspec.h"

describe(packedmap_construction) {

  it("Creates an empty packedmap") {
    PackedMap map = pmap_new(int);
    expect(map to not be_null);
    expect(map->begin to be_null);
    expect(map->size to be_zero);
    expect(map->capacity to be_zero);
    expect(map->element_size == sizeof(int));
    pmap_delete(&map);
    expect(map to be_null);
  }

  it("Creates a PackedMap with a specific reserve capacity") {
    PackedMap map = pmap_new_reserve(int, 12);
    expect(map to not be_null);
    expect(map->begin to not be_null);
    expect(map->size to be_zero);
    expect(map->capacity to equal(12));
    expect(map->element_size == sizeof(int));
    pmap_delete(&map);
    expect(map to be_null);
  }

}

describe(packedmap_add_item) {

  PackedMap map = pmap_new(int);
  slotkey_t key;

  it("requires a valid out-key parameter to emplace") {
    expect(to_assert);
    pmap_emplace(map, NULL);
  }

  it("requires a valid element to copy to insert") {
    expect(to_assert);
    pmap_insert(map, NULL);
  }

  it("emplaces a value into an empty pmap") {
    int* value = pmap_emplace(map, &key);
    expect(value to not be_null);
    expect(sk_unique(key) to not be_zero);
  }

  it("inserts a value into an empty pmap") {
    int value = 53;
    key = pmap_insert(map, &value);
    expect(sk_unique(key) to not be_zero);
  }

  context("adding items to a reserved pmap") {

    pmap_reserve(map, 10);
    expect(map->capacity, == , 10);

    it("emplaces a value into an existing pmap") {
      int* value = pmap_emplace(map, &key);
      expect(value to not be_null);
      expect(map->size to be_one);
    }

    it("inserts a value into an existing pmap") {
      int value = 12;
      key = pmap_insert(map, &value);
      expect(sk_unique(key) to not be_zero);
    }

  }

  after{
    expect(map->size to be_one);
    pmap_delete(&map);
  }

}

describe(packedmap_add_multiple) {

  PackedMap map = pmap_new(int);
  slotkey_t key;

  it("emplaces values into an empty map") {
    for (int i = 0; i < 10; ++i) {
      int* value = pmap_emplace(map, &key);
      expect(value to not be_null);
    }
  }

  it("inserts values into an empty map") {
    for (int i = 0; i < 40; ++i) {
      key = pmap_insert(map, &i);
      expect(sk_unique(key) to not be_zero);
    }
  }

  after{
    pmap_delete(&map);
  }

}

describe(packedmap_read) {

  PackedMap map = pmap_new(double);
  slotkey_t key;

  it("can read back a value that was inserted") {
    double* value = pmap_emplace(map, &key);
    *value = 37.1283;
    value = pmap_ref(map, key);
    expect(value to not be_null);
    expect(*value == 37.1283);
  }

  it("requires a valid read target") {
    expect(to_assert);
    key = SK_NULL;
    pmap_read(map, key, NULL);
  }

  it("copies a value when using 'read'") {
    double value = 5;
    key = pmap_insert(map, &value);
    expect(pmap_read(map, key, &value) to be_true);
    key = sk_build(sk_index(key) + 1, sk_unique(key));
    expect(pmap_read(map, key, &value) to be_false);
  }

  it("can check that a map contains a given key") {
    pmap_emplace(map, &key);
    expect(pmap_contains(map, key) to be_true);
    key = sk_build(sk_index(key) + 1, sk_unique(key));
    expect(pmap_contains(map, key) to be_false);
  }

  it("can retrieve the slotkey for a given index") {
    double values[] = { 1.11111111, 22.2222222, 333.333333 };
    pmap_insert(map, &values[0]);
    key = pmap_insert(map, &values[1]);
    pmap_insert(map, &values[2]);
    slotkey_t read_key = pmap_key(map, 1);
    expect(read_key to match(key));
    double* read_value = pmap_ref(map, read_key);
    expect(*read_value , == , values[1]);
  }

  after{
    pmap_delete(&map);
  }

}

describe(packedmap_remove) {

  PackedMap map = pmap_new(double);
  slotkey_t key;

  it("will ignore out of bounds keys") {
    key = sk_build(30, 0);
    expect(pmap_remove(map, key) to be_false);
  }

  it("can remove an item") {
    pmap_emplace(map, &key);
    expect(map->size to be_one);
    expect(pmap_remove to be_true given(map, key));
    expect(map->size to be_zero);
  }

  it("won't remove anything if the index is unoccupied") {
    pmap_emplace(map, &key);
    expect(map->size to be_one);
    key = sk_build(sk_index(key) + 1, sk_unique(key));
    expect(pmap_remove to be_false given(map, key));
    expect(map->size to be_one);
  }

  it("won't remove anything if the unique value doesn't match") {
    pmap_emplace(map, &key);
    expect(map->size to be_one);
    key = sk_build(sk_index(key), sk_unique(key) + 1);
    expect(pmap_remove to be_false given(map, key));
    expect(map->size to be_one);
  }

  it("removes an item before the last entry and keeps it contiguous") {
    double values[] = { 12345678.9, 999.87654321, 11111 };
    double expected[] = { 11111, 999.87654321 };

    slotkey_t key_first = pmap_insert(map, &values[0]);
    pmap_insert(map, &values[1]);
    slotkey_t key_last = pmap_insert(map, &values[2]);
    pmap_remove(map, key_first);
    expect(pmap_ref to be_null given(map, key_first));
    expect(map->size, == , 2);
    expect(view_size(map->view, sizeof(double)), == , 2);
    const double* read = pmap_ref(map, key_last);
    expect(read, == , map->begin);
    expect(*read, == , 11111);
    expect(map->end, == , read + 2);

    view_t out = view_range(expected, ARRAY_COUNT(expected), sizeof(double));
    expect(view_eq to be_true given(map->view, out));
  }

  context("when removing from a map with multiple items") {

    slotkey_t middle1 = { 0 };
    slotkey_t middle2 = { 0 };
    for (int i = 0; i < 40; ++i) {
      key = pmap_insert(map, &i);
      expect(sk_unique(key) to not be_zero);
      if (i == 12) middle1 = key;
      if (i == 34) middle2 = key;
    }

    it("can remove from the middle of the map") {
      pmap_remove(map, middle2);
      pmap_remove(map, middle1);
    }

    it("can remove from the middle of the map") {
      pmap_remove(map, middle2);
      pmap_remove(map, middle1);
      double test = 11;
      key = pmap_insert(map, &test);
      expect(key to not match(middle1));
      expect(key to not match(middle2));
    }

  }

  after {
    pmap_delete(&map);
  }

}

describe(packedmap_foreach) {

  int input_data[] = { 0xAAAAAA, 0xBBBBB, 0xCCCC, 0xDDD, 0xEE, 0xF };
  PackedMap map = pmap_new(int);
  for (size_t i = 0; i < ARRAY_COUNT(input_data); ++i) {
    pmap_insert(map, &input_data[i]);
  }

  it("will iterate through all the items in order") {
    int* test = &input_data[0];
    int* pmap_foreach(value, map) {
      expect(*value, == , *test);
      ++test;
    }
  }

  it("will iterate while providing a sequential index") {
    int* pmap_foreach_index(value, i, map) {
      expect(*value, == , input_data[i]);
    }
  }

  it("can iterate using keys") {
    int* pmap_foreach_kv(value, key, map) {
      expect(*value, == , input_data[sk_index(key)]);
    }
  }

  after{
    pmap_delete(&map);
  }

}

describe(packedmap_views) {

  view_t expected = view_empty;
  int input_data[] = { 0xAAAAAA, 0xBBBBB, 0xCCCC, 0xDDD, 0xEE, 0xF };
  PackedMap map = pmap_new(int);
  for (size_t i = 0; i < ARRAY_COUNT(input_data); ++i) {
    pmap_insert(map, &input_data[i]);
  }

  it("can iterate through the contiguous input data") {
    expected = view_range(input_data, ARRAY_COUNT(input_data), sizeof(int));
  }

  it("will shuffle elements around to ensure they stay contiguous") {
    int out[] = { 0xF, 0xBBBBB, 0xEE, 0xDDD };
    expected = view_range(out, ARRAY_COUNT(out), sizeof(int));
    expect(pmap_remove to be_true given(map, sk_build(0, 1))); // 0xAAAAAA
    expect(pmap_remove to be_true given(map, sk_build(2, 3))); // 0xCCCC
    expect(pmap_remove to be_false given(map, sk_build(2, 3)));
  }

  after {
    expect(view_eq to be_true given(map->view, expected));
    pmap_delete(&map);
  }

}

#define con_type int
#include "packedmap.h"
#undef con_type

describe(packedmap_specialized_type) {

  PackedMap_int ints = pmap_int_new();
  slotkey_t key;

  it("can add and remove using specialized functions") {
    *pmap_int_emplace(ints, &key) = 25;
    *pmap_int_emplace(ints, &key) = 67;
    *pmap_int_emplace(ints, &key) = 42;
    expect(ints->size , == , 3);
    pmap_int_remove(ints, key);
    expect(ints->size , == , 2);
    key = pmap_int_key(ints, 1);
    bool test = (*(pmap_int_ref(ints, key))) == 67;
    expect(test);
  }

  it("can add and remove items by value") {
    key = pmap_int_add(ints, 12);
    pmap_int_add(ints, 9484);
    pmap_int_add(ints, 99);
    expect(ints->size, == , 3);
    expect(pmap_int_get(ints, key), == , 12);
    pmap_int_clear(ints);
    expect(ints->size to be_zero);
    expect(ints->begin to not be_null);
  }

  after {
    pmap_int_delete(&ints);
    expect(ints == NULL);
  }

}

test_suite(tests_packedmap) {
  test_group(packedmap_construction),
  test_group(packedmap_add_item),
  test_group(packedmap_add_multiple),
  test_group(packedmap_read),
  test_group(packedmap_remove),
  test_group(packedmap_foreach),
  test_group(packedmap_views),
  test_group(packedmap_specialized_type),
  test_suite_end
};

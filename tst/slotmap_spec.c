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

#include "slotmap.h"

#define CSPEC_CUSTOM_TYPES \
  SlotMap: "SlotMap"

#include "cspec.h"

describe(slotmap_construction) {

  it("Creates an empty slotmap") {
    SlotMap map = smap_new(int);
    expect(map to not be_null);
    expect(map->size to be_zero);
    expect(map->capacity to be_zero);
    expect(map->element_size == sizeof(int));
    smap_delete(&map);
    expect(map to be_null);
  }

  it("Creates a slotmap with a specific reserve capacity") {
    SlotMap map = smap_new_reserve(int, 12);
    expect(map to not be_null);
    expect(map->size to be_zero);
    expect(map->capacity to equal(12));
    expect(map->element_size == sizeof(int));
    smap_delete(&map);
    expect(map to be_null);
  }

}

describe(slotmap_add_item) {

  SlotMap map = smap_new(int);
  slotkey_t key;

  it("requires a valid out-key parameter to emplace") {
    expect(to_assert);
    smap_emplace(map, NULL);
  }

  it("requires a valid element to copy to insert") {
    expect(to_assert);
    smap_insert(map, NULL);
  }

  it("emplaces a value into an empty map") {
    int* value = smap_emplace(map, &key);
    expect(value to not be_null);
    expect(key.unique to not be_zero);
  }

  it("inserts a value into an empty map") {
    int value = 53;
    key = smap_insert(map, &value);
    expect(key.unique to not be_zero);
  }

  context("adding items to a reserved map") {

    smap_reserve(map, 10);
    expect(map->capacity , == , 10);

    it("emplaces a value into an existing map") {
      int* value = smap_emplace(map, &key);
      expect(value to not be_null);
      expect(map->size to be_one);
    }

    it("inserts a value into an existing map") {
      int value = 12;
      key = smap_insert(map, &value);
      expect(key.unique to not be_zero);
    }

  }

  after {
    expect(map->size to be_one);
    smap_delete(&map);
  }

}

describe(slotmap_add_multiple) {

  SlotMap map = smap_new(int);
  slotkey_t key;

  it("emplaces a value into an empty map") {
    for (int i = 0; i < 10; ++i) {
      int* value = smap_emplace(map, &key);
      expect(value to not be_null);
    }
  }

  it("inserts a value into an empty map") {
    for (int i = 0; i < 40; ++i) {
      key = smap_insert(map, &i);
      expect(key.unique to not be_zero);
    }
  }

  after{
    smap_delete(&map);
  }

}

describe(slotmap_read) {

  SlotMap map = smap_new(double);
  slotkey_t key;

  it("can read back a value that was inserted") {
    double* value = smap_emplace(map, &key);
    *value = 37.1283;
    value = smap_ref(map, key);
    expect(*value == 37.1283);
  }

  it("requires a valid read target") {
    expect(to_assert);
    key = (slotkey_t){ 0, 0 };
    smap_read(map, key, NULL);
  }

  it("copies a value when using 'read'") {
    double value = 5;
    key = smap_insert(map, &value);
    expect(smap_read(map, key, &value) to be_true);
    ++key.index;
    expect(smap_read(map, key, &value) to be_false);
  }

  after{
    smap_delete(&map);
  }

}

describe(slotmap_remove) {

  SlotMap map = smap_new(double);
  slotkey_t key;

  it("will ignore out of bounds keys") {
    key.index = 30;
    expect(smap_remove(map, key) to be_false);
  }

  it("can remove an item") {
    smap_emplace(map, &key);
    expect(map->size to be_one);
    smap_remove(map, key);
    expect(map->size to be_zero);
  }

  it("won't remove anything if the index is unoccupied") {
    smap_emplace(map, &key);
    expect(map->size to be_one);
    ++key.index;
    smap_remove(map, key);
    expect(map->size to be_one);
  }

  it("won't remove anything if the unique value doesn't match") {
    smap_emplace(map, &key);
    expect(map->size to be_one);
    ++key.unique;
    smap_remove(map, key);
    expect(map->size to be_one);
  }

  after{
    smap_delete(&map);
  }

}

test_suite(tests_slotmap) {
  test_group(slotmap_construction),
  test_group(slotmap_add_item),
  test_group(slotmap_add_multiple),
  test_group(slotmap_read),
  test_group(slotmap_remove),
  test_suite_end
};

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

#include "map.h"

#define CSPEC_CUSTOM_TYPES  \
  HMap: "HMap"

#include "cspec.h"

int cmp_int(const int* a, const int* b) { return a < b; }

describe(map_construction) {

  it("Takes no action when trying to delete a null pointer") {
    HMap map = NULL;
    map_delete(&map);
    map_delete(NULL);
    expect(malloc_count == 0);
    expect(free_count == 0);
  }

  it("Throws an assert when out of memory") {
    expect(malloc_to_fail);
    expect(to_assert);
    map_new(int, int, NULL, NULL);
  }

  it("Creates and deletes a map without any inserts or reads") {
    HMap map = map_new(int, int, NULL, NULL);
    expect(map to not be_null);
    expect(map->size to be_zero);
    expect(map->element_size == sizeof(int));
    expect(map->key_size == sizeof(int));
    map_delete(&map);
    expect(map to be_null);
  }

}

#include "slice.h"

describe(map_string_keys) {

  HMap map = map_new(slice_t, int, slice_compare_vptr, slice_hash_vptr);

  it("can use slices to index") {
    int* value = map_emplace(map, &S("FIRST"));
    expect(value to not be_null);
    *value = 12;

    int* retrieved = map_ref(map, &S("FIRST"));
    expect(value to equal(retrieved));
    expect(*value to equal(*retrieved));

    retrieved = map_ref(map, &S("first"));
    expect(retrieved to be_null);
  }

  after{
    map_delete(&map);
  }

}

describe(map_ensure) {

  HMap map = map_new(int, int, NULL, NULL);

  context("Critical failure cases") {

    it("throws an assert when given a NULL key") {
      expect(to_assert);
      map_ensure(map, NULL);
    }

    it("throws an assert when given a NULL map") {
      expect(to_assert);
      map_ensure(NULL, &(int){ 1 });
    }

  }

  it("will initialize a default capacity of 8 when adding to an empty map") {
    expect(map->size to be_zero);
    expect(map->capacity to be_zero);

    map_ensure(map, &(int){ 5 });

    expect(map->size to be_one);
    expect(map->capacity, == , 8);
  }

  it("adds an element to the map and reads it back") {
    ensure_t result = map_ensure(map, &(int){ 5 });

    expect(result.is_new to be_true);
    expect(map->size to equal(1));

    *(int*)result.value = 12;
    result = map_ensure(map, &(int){ 0xBADF00D5 });

    expect(result.is_new to be_true);
    expect(map->size to equal(2));

    *(int*)result.value = 37;

    int* first = map_ref(map, &(int){ 5 });
    int* second = map_ref(map, &(int){ 0xBADF00D5 });

    expect(*first to equal(12));
    expect(*second to equal(37));
  }

  it("grows the map when it reaches 75% capacity") {
    int i;
    for (i = 1; i < 6; ++i) {
      *(int*)map_ensure(map, &i).value = i * 5;
      expect(map->size, == , (index_t)i);
    }

    // validate refernce values
    for (i = 1; i < 6; ++i) {
      int* result = map_ref(map, &i);
      expect(*result to equal(i * 5));
    }

    expect(map->capacity, == , 8);
    *(int*)map_ensure(map, &i).value = i * 5;
    expect(map->capacity, == , 16);

    // validate values again to make sure they moved
    for (i = 1; i < 7; ++i) {
      int* result = map_ref(map, &i);
      expect(*result to equal(i * 5));
    }
  }

  it("returns the existing element when a duplicate key is used") {
    int key = 1337;
    *(int*)map_ensure(map, &key).value = 123;

    ensure_t find = map_ensure(map, &key);
    expect(find.is_new to be_false);
    expect(*(int*)find.value to equal(123));
  }

  after {
    map_delete(&map);
  }

}

describe(map_emplace) {

  HMap map = map_new(int, int, NULL, NULL);

  context("critical failure cases") {

    expect(to_assert);

    it("asserts when given a NULL key") {
      map_emplace(map, NULL);
    }

    it("asserts when given a NULL map") {
      map_emplace(NULL, &(int){ 5 });
    }

  }

  it("will return NULL when re-using a key already in the map") {
    int key = 1337;
    *(int*)map_emplace(map, &key) = 123;
    expect(map->size to be_one);

    int* result = map_emplace(map, &key);
    expect(result to be_null);
  }

  after {
    map_delete(&map);
  }

}

describe(map_ref) {

  HMap map = map_new(int, int, NULL, NULL);

  it("asserts if a NULL vlaue is given for the key") {
    expect(to_assert);
    map_ref(map, NULL);
  }

  it("returns false if the key is not found in the map") {
    int* ptr = map_ref(map, &(int){ 5 });
    expect(ptr to be_null);
  }

  after {
    map_delete(&map);
  }

}

describe(map_remove) {

  HMap map = map_new(int, int, NULL, NULL);

  it("asserts if a NULL vlaue is given for the key") {
    expect(to_assert);
    map_remove(map, NULL);
  }

  it("returns false when removing a key that isn't in the map") {
    bool result = map_remove(map, &(int){ 5 });
    expect(result to be_false);
  }

  it("can remove a value from the map") {
    map_emplace(map, &(int){ 5 });
    expect(map->size to be_one);
    expect(map_remove to be_true given(map, &(int){ 5 }));
    expect(map->size to be_zero);
  }

  context("when values are stacked into a bucket") {

    int key_2 = 2; // 2 and 4 have conflicting hashes for an 8 slot container
    int key_4 = 4;
    *(int*)map_emplace(map, &key_2) = 10;
    *(int*)map_emplace(map, &key_4) = 20;
    expect(map->size to equal(2));

    it("will remove values from the root of the bucket without affecting others") {
      expect(map_remove to be_true given(map, &key_2));
      int* value = map_ref(map, &key_4);
      expect(value to not be_null);
      expect(*value to equal(20));
      expect(map_ref to be_null given(map, &key_2));
    }

    it("will remove values later in the bucket without affecting the root") {
      expect(map_remove to be_true given(map, &key_4));
      int* value = map_ref(map, &key_2);
      expect(value to not be_null);
      expect(*value to equal(10));
      expect(map_ref to be_null given(map, &key_4));
    }

  }

  after {
    map_delete(&map);
  }

}





/*
// Defining element and key types
#define con_type float
#define key_type byte
#define con_cmp cmp_int
#include "map.h"
#undef con_type
#undef key_type
#undef con_cmp
*/
// Define with default key type (span)
#define con_type int
#define con_prefix int
#include "map.h"
#undef con_type
#undef con_prefix


describe(map_stuff) {

  HMap_int ints = map_int_new();

  ensure_int_t result = map_int_ensure(ints, S("Input"));
  *result.value = 12;

  map_int_new();
  map_int_ref(ints, S("Test"));

  map_int_delete(&ints);


  /*
  kv_byte_float_t t;
  inlist_kv_byte_float_t i;
  inlist_kv_byte_float_t n;
  inlist_byte_t keys;
  inlist_byte_t intinlist;
  HMap_byte_float m;
  HMap_int p;

  byte bkey = m->items->key;
  float bvalue = m->items->value;
  const kv_byte_float_t* kvbf = m->items->next;
  const inlist_byte_t* asdf = m->keys;

  //const slice_t* list_foreach(key in m->keys)
  

  slice_t slice = p->items->key;
  int ivalue = p->items->value;

  //HMap_int intmap;

  //*/
}

test_suite(tests_map) {
  test_group(map_construction),
  test_group(map_string_keys),
  test_group(map_ensure),
  test_group(map_emplace),
  test_group(map_ref),
  test_group(map_remove),
  test_group(map_stuff),
  test_suite_end
};

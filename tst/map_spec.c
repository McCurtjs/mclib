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

static int cmp_int_void(const void* a, const void* b) {
  return *(int*)a - *(int*)b;
}

int cmp_int(const int* a, const int* b);

// Defining element and key types
#define con_type float
#define key_type byte
#define con_cmp cmp_int
#include "map.h"
#undef con_type
#undef key_type
#undef con_cmp

// Define with default key type (span)
#define con_type int
#define con_prefix int
#include "map.h"
#undef con_type
#undef con_prefix

#include "cspec.h"

describe(map_stuff) {
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
  test_group(map_stuff),
  test_suite_end
};

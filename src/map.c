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

#include "map.h"
#include "array_byte.h"

typedef struct keylist_internal_t {
  inlist_node_t node;
  hash_t hash;
} keylist_internal_t;

typedef struct Map_Cell {
  union {
    inlist_node_t bucket_list_node;
    inlist_node_t free_list_node;
    size_t flags; // lsb flag for list type
  };
  hash_t hash;
  inlist_node_t kvlist_node;
  // struct kvpair
  //   key
  //   value
} Map_Cell;

// internal opaque structure:
typedef struct Map_Internal {
  // public (read only)
  index_t size;
  index_t capacity;
  index_t key_size;
  index_t element_size;

  // public with specified types
  inlist_t items;

  // private
  inlist_t free_list_head;
  inlist_t item_list_head;

  key_compare_fn compare_keys;
  key_hash_fn hash_key;

  Array_byte data;
  // Array_Internal size space to avoid extra jump?
} Map_Internal;



// Change base type from HMap to Map_base or Map_void? Map_data?
// Use something like vmap or imap for map-implementation? 



/*



[ b/f next , b/f prev , hash , kvnext , kvprev , key , value ]




*/




/*

[ [bucket/free|active], K, V ]

[ next_0 , key  , value ]
[ free_1 , ...  , ...   ]

[  ]

[  ]



[]



*/




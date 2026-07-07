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

#define MCLIB_INTERNAL_IMPL
#include "data_tree.h"

#include "array_byte.h"

#include <stdlib.h>
#include <string.h>

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
#endif

#define con_type struct dnode_t
#define con_prefix dnode
#include "array.h"
#undef con_prefix
#undef con_type

#define con_type struct dnode_member_t
#define con_prefix dmemb
#include "array.h"
#undef con_prefix
#undef con_type

#define con_type slice_t
#define con_prefix slice
#include "map.h"
#undef con_prefix
#undef con_type

typedef struct DataTree_Internal {
  struct _opaque_DataTree_t pub;

  //array_dnode_t nodes;
  //array_dmemb_t membs;
  //array_byte_t  data;
} DataTree_Internal;

DataTree_Internal* _dtree_new(void) {
  // need to zero-initialize to ensure we pass valid empty arrays to init
  DataTree_Internal* ret = calloc(1, (sizeof(*ret)));
  assert(ret);

  //arr_dnode_init_reserve(&ret->nodes, 1);
  //arr_dmemb_init(&ret->membs);
  //arr_byte_init(&ret->data);
  ret->pub.root = NULL;

  return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Create and copy
////////////////////////////////////////////////////////////////////////////////

static void _dtree_copy_node_contents(DataTree_Internal*, DataNode, DataView);

static slice_t _dtree_store_string(DataTree_Internal* tree, slice_t slice) {
  UNUSED(tree);
  // needs to be updated to store strings in a set and arena
  //    (set will contain slices and hash, arena will store the string data)
  char* str = malloc(slice.length + 1);
  assert(str);
  memcpy(str, slice.begin, slice.size);
  str[slice.size] = '\0';
  return slice_build(str, slice.size);
}

////////////////////////////////////////////////////////////////////////////////

static void _dtree_copy_object(
  DataTree_Internal* tree, DataNode node, DataView src
) {
  assert(src->object.size >= 0);
  node->object.size = src->object.size;

  if (src->object.size <= 0) {
    node->object.children = NULL;
    return;
  }

  dnode_member_t* children = malloc(sizeof(dnode_member_t) * src->object.size);
  assert(children);

  for (index_t i = 0; i < src->object.size; ++i) {
    dview_member_t* src_child = &src->object.children[i];
    children[i].name = _dtree_store_string(tree, src_child->name);
    _dtree_copy_node_contents(tree, &children[i].node, &src_child->node);
  }

  node->object.children = children;
}

////////////////////////////////////////////////////////////////////////////////

static void _dtree_copy_array(
  DataTree_Internal* tree, DataNode node, DataView src
) {
  assert(src->array.size >= 0);
  assert(src->array.elem_type >= 0);
  assert(src->array.elem_type <= DN_ARRAY_ELEM_MIXED);

  node->array.elem_type = src->array.elem_type;
  node->array.size = src->array.size;
  node->array.nodes = NULL;

  index_t size_bytes = src->array.size;

  if (size_bytes <= 0) return;

  switch (src->array.elem_type) {
    case DN_NULL:   return;

    case DN_BOOL:   size_bytes *= sizeof(bool);     break;
    case DN_INT:    size_bytes *= sizeof(int64_t);  break;
    case DN_FLOAT:  size_bytes *= sizeof(double);   break;
    case DN_STRING: size_bytes *= sizeof(slice_t);  break;

    case DN_ARRAY_ELEM_MIXED: SWITCH_FALLTHROUGH;
    case DN_ARRAY:            SWITCH_FALLTHROUGH;
    case DN_OBJECT: size_bytes *= sizeof(dnode_t);  break;

    default: assert(false); break;
  }

  node->array.nodes = malloc(size_bytes);
  assert(node->array.nodes);
  memcpy(node->array.nodes, src->array.nodes, size_bytes);

  if (src->array.elem_type == DN_OBJECT
  ||  src->array.elem_type == DN_ARRAY
  ||  src->array.elem_type == DN_ARRAY_ELEM_MIXED
  ) {
    for (index_t i = 0; i < src->array.size; ++i) {
      DataNode arr_target = &node->array.nodes[i];
      DataView arr_source = &src->array.nodes[i];
      _dtree_copy_node_contents(tree, arr_target, arr_source);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

static void _dtree_copy_node_contents(
  DataTree_Internal* tree, DataNode node, DataView src
) {
  node->type = src->type;

  switch (src->type) {
    case DN_NULL:   break;
    case DN_BOOL:   node->value_bool = src->value_bool;   break;
    case DN_INT:    node->value_int = src->value_int;     break;
    case DN_FLOAT:  node->value_float = src->value_float; break;

    case DN_STRING: {
      node->value_str = _dtree_store_string(tree, src->value_str);
    } break;

    case DN_OBJECT: _dtree_copy_object(tree, node, src); break; 

    case DN_ARRAY: _dtree_copy_array(tree, node, src); break;

    default: assert(false); break;
  }
}

////////////////////////////////////////////////////////////////////////////////

static DataNode _dtree_copy_node(DataTree_Internal* tree, DataView src) {
  assert(src->type >= 0 && src->type < DN_ARRAY_ELEM_MIXED);

  DataNode node = malloc(sizeof(dnode_t));
  assert(node);
  _dtree_copy_node_contents(tree, node, src);

  return node;
}

////////////////////////////////////////////////////////////////////////////////

static void _dtree_consolidate_strings(DataTree_Internal* tree) {
  UNUSED(tree);

  HMap_slice strs = map_slice_new();

  // for each node:
  //    if it's a string node, add string to map as { src, empty }
  //    if it's an object, add all its member keys to the map as well
  //    on each added string, add length to a counter if it's new
  //    (use node array as stack to avoid recursion?)
  // 
  // allocate the space for all the strings at once
  // 
  // for each string in map:
  //    copy the key into data and update map value as { key: src, value: copy }
  // 
  // for each node:
  //    run through tree and swap all strings with their copies

  map_slice_delete(&strs);
}

////////////////////////////////////////////////////////////////////////////////

DataTree dtree_copy(const DataView other) {
  assert(other);

  // need to walk the tree first to count nodes and members to avoid
  // array resizing... any time the data array resizes, all pointers break
  //
  // OR - restructure the code so it pre-order reserves, calls to create
  //    child nodes, then post-order creates the node, links and returns?

  DataTree_Internal* ret = _dtree_new();
  ret->pub.root = _dtree_copy_node(ret, other);

  // copy all the data that still references the source view
  _dtree_consolidate_strings(ret);

  return (DataTree)ret;
}

////////////////////////////////////////////////////////////////////////////////
// Delete tree
////////////////////////////////////////////////////////////////////////////////

void _dtree_delete_node(DataNode node) {

  switch (node->type) {
    case DN_OBJECT: {
      for (index_t i = 0; i < node->object.size; ++i) {
        _dtree_delete_node(&node->object.children[i].node);
        free((char*)node->object.children[i].name.begin);
      }

      if (node->object.size > 0) {
        free(node->object.children);
      }
    } break;

    case DN_ARRAY: {
      dnode_type_t type = node->array.elem_type;
      if (type == DN_OBJECT
      ||  type == DN_ARRAY
      ||  type == DN_ARRAY_ELEM_MIXED
      ) {
        for (index_t i = 0; i < node->array.size; ++i) {
          _dtree_delete_node(&node->array.nodes[i]);
        }
      }
      else if (type == DN_STRING) {
        for (index_t i = 0; i < node->array.size; ++i) {
          free((char*)node->array.strings[i].begin);
        }
      }

      if (node->array.size > 0) {
        free(node->array.nodes);
      }
    } break;

    case DN_STRING: {
      free((char*)node->value_str.begin);
    } break;

    default: break;
  }

}

////////////////////////////////////////////////////////////////////////////////

void dtree_delete(DataTree* p_dtree) {
  if (!p_dtree || !*p_dtree) return;
  DataTree_Internal* dtree = (DataTree_Internal*)*p_dtree;

  if (dtree->pub.root) {
    _dtree_delete_node(dtree->pub.root);
    free(dtree->pub.root);
  }
  free(dtree);


  //Array_dnode nodes = &dtree->nodes;
  //Array_byte data = &dtree->data;
  //arr_dnode_delete(&nodes);
  //arr_byte_delete(&data);
  *p_dtree = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Load tree from JSON
////////////////////////////////////////////////////////////////////////////////

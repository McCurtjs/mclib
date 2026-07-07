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
#include "data_node.h"

#include "str.h"
#include "array_byte.h"

#include <stdlib.h>
#include <math.h> // isnan, isfinite

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
#endif

////////////////////////////////////////////////////////////////////////////////
// Helper functions for accessing nodes/values from data_view.c
////////////////////////////////////////////////////////////////////////////////

// Rules for casting node values between types
bool      _dnode_coerce_null(dnode_value_t, DataNode out);
bool      _dnode_coerce_bool(dnode_value_t, bool* out);
bool      _dnode_coerce_int(dnode_value_t, int64_t* out);
bool      _dnode_coerce_float(dnode_value_t, double* out);
bool      _dnode_coerce_string(dnode_value_t, slice_t* out);
bool      _dnode_coerce_value_node(dnode_value_t, DataNode out);

// Read the value of 
bool      _dnode_read_value(DataNode, dnode_value_t* out);

// Get the child node of an object by its key name
DataNode  _dnode_key_into_object(DataNode node, slice_t key);

////////////////////////////////////////////////////////////////////////////////
// Select logic
////////////////////////////////////////////////////////////////////////////////

void _dnode_select_object(DataNode node, DataNode query) {
  assert(node);
  assert(query);
  assert(node->type == DN_OBJECT);
  assert(query->type == DN_OBJECT);

  // check for values on each member of the query selector
  for (index_t i = 0; i < query->object.size; ++i) {
    dnode_member_t* query_member = &query->object.children[i];

    // linear search the data source for matching members
    // currently, there's no guarantee that members are sorted by name
    DataNode child = _dnode_key_into_object(node, query_member->name);
    if (!child) continue;

    dnode_select(child, &query_member->node);
  }
}

void _dnode_select_array(DataNode node, DataNode query) {
  assert(node);
  assert(query);
  assert(node->type == DN_ARRAY);
  assert(query->type == DN_ARRAY);

  index_t size = MIN(query->array.size, node->array.size);

  for (index_t i = 0; i < size; ++i) {

    // construct a proxy source node to use with our copy functions
    dnode_value_t source = { .type = node->array.elem_type };

    switch (node->array.elem_type) {

      case DN_ARRAY:  SWITCH_FALLTHROUGH;
      case DN_OBJECT: SWITCH_FALLTHROUGH;
      case DN_ARRAY_ELEM_MIXED:
        _dnode_read_value(&node->array.nodes[i], &source);
        break;

      case DN_BOOL:   source.value_bool   = &node->array.bools[i];    break;
      case DN_INT:    source.value_int    = &node->array.ints[i];     break;
      case DN_FLOAT:  source.value_float  = &node->array.floats[i];   break;
      case DN_STRING: source.value_str    = &node->array.strings[i];  break;

      // if a type doesn't match any selector, it's not supported yet
      default: continue;
    }

    // apply the array values to the query selector via the proxy
    switch (query->array.elem_type) {

      case DN_ARRAY:  SWITCH_FALLTHROUGH;
      case DN_OBJECT: SWITCH_FALLTHROUGH;
      case DN_ARRAY_ELEM_MIXED:

        if (source.type == DN_ARRAY || source.type == DN_OBJECT) {
          dnode_select(source.node, &query->array.nodes[i]);
        }
        else {
          _dnode_coerce_value_node(source, &query->array.nodes[i]);
        }
        break;

      case DN_BOOL:
        _dnode_coerce_bool(source, &query->array.bools[i]);
        break;

      case DN_INT:
        _dnode_coerce_int(source, &query->array.ints[i]);
        break;

      case DN_FLOAT:
        _dnode_coerce_float(source, &query->array.floats[i]);
        break;

      case DN_STRING:
        _dnode_coerce_string(source, &query->array.strings[i]);
        break;

      default: break;

    }
  }
}

DataNode dnode_select(DataNode node, DataNode query) {
  assert(node);
  assert(query);

  dnode_value_t value;
  if (!_dnode_read_value(node, &value)) return query;

  switch (query->type) {

    // Treat null query nodes as "wildcards" and read in both the type and value
    case DN_NULL:   _dnode_coerce_null(value, query);                   break;

    // Query checking for boolean (or convertible) value
    case DN_BOOL:   _dnode_coerce_bool(value, &query->value_bool);      break;

    // Query checking for integer (or convertible) value
    case DN_INT:    _dnode_coerce_int(value, &query->value_int);        break;

    // Query checking for floating point (or convertible) value
    case DN_FLOAT:  _dnode_coerce_float(value, &query->value_float);    break;

    // Query checking for string value
    case DN_STRING: _dnode_coerce_string(value, &query->value_str);     break;

    // Query recursively checking for sub-objects
    case DN_OBJECT:
      if (node->type == DN_OBJECT) _dnode_select_object(node, query);
      break;

    // Query recursively checking for sub-arrays
    case DN_ARRAY:
      if (node->type == DN_ARRAY) _dnode_select_array(node, query);
      break;

    // No type matched - this should only happen if new supported types are
    //    added, but forgotten about.
    default:
      assert(false);
      break;

  }

  return query;
}

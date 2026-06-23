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

#include "array_byte.h"

#include <stdlib.h>

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
#endif

////////////////////////////////////////////////////////////////////////////////
// Serialization to JSON
////////////////////////////////////////////////////////////////////////////////

String dnode_to_json(DataNode root) {
  UNUSED(root);
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Access
////////////////////////////////////////////////////////////////////////////////

static inline bool _valid_node_type(dnode_type_t type) {
  return type >= 0 && type < DN_ARRAY_ELEM_MIXED;
}

////////////////////////////////////////////////////////////////////////////////

static inline bool _dnode_read_value(DataNode node, dnode_value_t* out) {
  assert(node);

  switch (node->type) {
    case DN_OBJECT:   SWITCH_FALLTHROUGH;
    case DN_ARRAY:
      if (out) *out = (dnode_value_t) { .type = node->type, .node = node };
      break;
    case DN_BOOL:     SWITCH_FALLTHROUGH;
    case DN_INT:      SWITCH_FALLTHROUGH;
    case DN_FLOAT:    SWITCH_FALLTHROUGH;
    case DN_STRING:
      if (out) *out = (dnode_value_t) {
        .type = node->type, .value_int = &node->value_int
      };
      break;
    case DN_NULL:
      if (out) *out = (dnode_value_t) { .type = DN_NULL };
      break;
    default:
      // invalid node type
      assert(false);
      return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

static inline bool _success_with_value(
  dnode_type_t type, void* value, dnode_value_t* out_value
) {
  if (out_value) {
    // all the pointers are in a union, so which member doesn't actually matter
    *out_value = (dnode_value_t){ .type = type, .node = value };
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool _dnode_read_next(DataNode, slice_t path, dnode_value_t* out);

// Gets a child node of an object based on its name
static DataNode _dnode_key_into_object(DataNode node, slice_t key) {
  if (node->type != DN_OBJECT) return NULL;

  assert(node->object.size >= 0);

  for (index_t i = 0; i < node->object.size; ++i) {
    dnode_member_t* member = &node->object.children[i];
    if (slice_eq(key, member->name)) {
      return &member->node;
    }
  }

  return NULL;
}

// Index into an object or array after hitting an open bracket
// Expects to have already read the opening '[' character
static bool _dnode_read_index(DataNode node, slice_t path, dnode_value_t* out) {
  assert(node);

  path = slice_trim_start(path);

  // key-based indexing into an object
  if (path.begin[0] == '\'') {
    if (node->type != DN_OBJECT) return false;
    partition_slice_t split = slice_partition_char(slice_drop(path, 1), S("'"));

    // non-terminating quote case
    if (split.delimiter.size == 0) return false;

    DataNode child = _dnode_key_into_object(node, split.left);

    // no child with this key
    if (!child) return false;

    // validate that there's a closing bracket
    path = slice_trim_start(split.right);
    if (path.size <= 0 || path.begin[0] != ']') return false;
    path = slice_drop(path, 1);

    return _dnode_read_next(child, path, out);
  }

  // number-based index into an array
  else {
    index_t index;

    if (node->type != DN_ARRAY) return false;
    partition_slice_t part = slice_partition_char(path, S("]"));

    // missing closing bracket
    if (part.delimiter.size == 0) return false;

    // contents of brackets is not parsable as a number
    if (!slice_to_long(part.left, &index)) return false;

    // support negative indexing
    if (index < 0) index += node->array.size;

    // index is out of bounds
    if (index < 0 || index >= node->array.size) return false;

    // determine how to evaluate the index result
    switch (node->array.elem_type) {
      case DN_OBJECT: SWITCH_FALLTHROUGH;
      case DN_ARRAY:  SWITCH_FALLTHROUGH;
      case DN_ARRAY_ELEM_MIXED:
        return _dnode_read_next(&node->array.nodes[index], part.right, out);

      case DN_NULL:
        return _success_with_value(DN_NULL, NULL, out);

      case DN_BOOL:
        return _success_with_value(DN_BOOL, &node->array.bools[index], out);

      case DN_INT:
        return _success_with_value(DN_INT, &node->array.ints[index], out);

      case DN_FLOAT:
        return _success_with_value(DN_FLOAT, &node->array.floats[index], out);

      case DN_STRING:
        return _success_with_value(DN_STRING, &node->array.strings[index], out);

      default:
        return false;
    }
  }

  // attempting to index into a non-indexable type
  return false;
}

// Index into an object using the member value "dot" syntax
// Assumes previous character was a '.' accessing into an object node
static bool _dnode_read_member(DataNode obj, slice_t path, dnode_value_t* out) {
  assert(obj);

  if (obj->type == DN_OBJECT) {
    assert(obj->object.size >= 0);
    index_t next_pos = slice_index_of_char(path, S(".["));

    // handles either a trailing dot, double dot, or ".[" case
    if (next_pos <= 0) return false;

    pair_slice_t split = slice_split_at(path, next_pos);

    slice_t key = slice_trim(split.left);
    DataNode child = _dnode_key_into_object(obj, key);
    if (!child) return false;

    return _dnode_read_next(child, split.right, out);
  }

  // can't use '.' accessor on anything other than an object (for now)
  return false;
}

// Check for the next node type, or if we've finished reading the path
static bool _dnode_read_next(DataNode node, slice_t path, dnode_value_t* out) {
  path = slice_trim_start(path);

  if (slice_is_empty(path)) {
    return _dnode_read_value(node, out);
  }

  if (path.begin[0] == '.') {
    return _dnode_read_member(node, slice_drop(path, 1), out);
  }

  if (path.begin[0] == '[') {
    return _dnode_read_index(node, slice_drop(path, 1), out);
  }

  return false;
}

// Start reading the node path, check for leading '$' root object
bool dnode_read(DataNode node, slice_t path, dnode_value_t* out_value) {
  assert(node);
  assert(slice_is_valid(path));

  path = slice_trim(path);

  if (slice_starts_with(path, S("$"))) {
    return _dnode_read_next(node, slice_drop(path, 1), out_value);
  }

  if (slice_is_empty(path)) {
    return _dnode_read_value(node, out_value);
  }

  // allow omitting the $ and assume the path starts at an object
  return _dnode_read_member(node, path, out_value);
}

////////////////////////////////////////////////////////////////////////////////

bool dnode_contains(const DataNode node, slice_t path) {
  return dnode_read(node, path, NULL);
}

bool* dnode_ref_bool(DataNode node, slice_t path) {
  dnode_value_t value;
  if (dnode_read(node, path, &value) && value.type == DN_BOOL)
    return value.value_bool;
  return NULL;
}

int64_t* dnode_ref_int(DataNode node, slice_t path) {
  dnode_value_t value;
  if (dnode_read(node, path, &value) && value.type == DN_INT)
    return value.value_int;
  return NULL;
}

double* dnode_ref_float(DataNode node, slice_t path) {
  dnode_value_t value;
  if (dnode_read(node, path, &value) && value.type == DN_FLOAT)
    return value.value_float;
  return NULL;
}

slice_t* dnode_ref_str(DataNode node, slice_t path) {
  dnode_value_t value;
  if (dnode_read(node, path, &value) && value.type == DN_STRING)
    return value.value_str;
  return NULL;
}

DataNode dnode_ref_object(DataNode node, slice_t path) {
  dnode_value_t value;
  if (dnode_read(node, path, &value) && value.type == DN_OBJECT)
    return value.node;
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

DataNode dnode_select(const DataNode node, DataNode query) {
  UNUSED(node);
  return query;
}

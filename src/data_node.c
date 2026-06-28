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
#include <math.h> // isnan, isfinite

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
// Rules for casting node values between types
////////////////////////////////////////////////////////////////////////////////

static bool _dnode_coerce_null(dnode_value_t value, DataNode out) {
  assert(value.type >= 0 && value.type < DN_ARRAY_ELEM_MIXED);
  assert(out);
  assert(out->type == DN_NULL);

  switch (value.type) {

    case DN_BOOL:   *out = NODE_BOOL(*value.value_bool);    break;
    case DN_INT:    *out = NODE_INT(*value.value_int);      break;
    case DN_FLOAT:  *out = NODE_FLOAT(*value.value_float);  break;
    case DN_STRING: *out = NODE_STRING(*value.value_str);   break;

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool _dnode_coerce_bool(dnode_value_t value, bool* out) {
  assert(value.type >= 0 && value.type < DN_ARRAY_ELEM_MIXED);
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool;               break;
    case DN_INT:    *out = *value.value_int != 0;           break;
    case DN_FLOAT:
      // NaN is false, INF and non-zero is true
      *out = !isnan(*value.value_float) && *value.value_float != 0;
      break;
    case DN_STRING: slice_to_bool(*value.value_str, out);   break;

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool _dnode_coerce_int(dnode_value_t value, int64_t* out) {
  assert(value.type >= 0 && value.type < DN_ARRAY_ELEM_MIXED);
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool ? 1 : 0;       break;
    case DN_INT:    *out = *value.value_int;                break;
    case DN_FLOAT:
      // perform no conversion on INF or NaN values (retains defaults)
      if (!isfinite(*value.value_float)) return false;
      *out = (int64_t)*value.value_float;
      break;
    case DN_STRING: slice_to_long(*value.value_str, out);   break;

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool _dnode_coerce_float(dnode_value_t value, double* out) {
  assert(value.type >= 0 && value.type < DN_ARRAY_ELEM_MIXED);
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool ? 1 : 0;       break;
    case DN_INT:    *out = (double)*value.value_int;        break;
    case DN_FLOAT:  *out = *value.value_float;              break;
    case DN_STRING: slice_to_double(*value.value_str, out); break;

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool _dnode_coerce_string(dnode_value_t value, slice_t* out) {
  assert(value.type >= 0 && value.type < DN_ARRAY_ELEM_MIXED);
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool ? slice_true : slice_false;  break;
    case DN_STRING: *out = *value.value_str;                              break;

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

static bool _dnode_coerce_value_node(dnode_value_t value, DataNode out) {
  assert(value.type >= 0 && value.type < DN_ARRAY_ELEM_MIXED);
  assert(out);

  switch (out->type) {

    case DN_NULL:   return _dnode_coerce_null(value, out);
    case DN_BOOL:   return _dnode_coerce_bool(value, &out->value_bool);
    case DN_INT:    return _dnode_coerce_int(value, &out->value_int);
    case DN_FLOAT:  return _dnode_coerce_float(value, &out->value_float);
    case DN_STRING: return _dnode_coerce_string(value, &out->value_str);

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Access helpers
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
// Parsing logic for accessor path strings
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

bool dnode_contains(DataNode node, slice_t path) {
  return dnode_read(node, path, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// Typed accessors
////////////////////////////////////////////////////////////////////////////////

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

DataNode dnode_ref_array(DataNode node, slice_t path) {
  dnode_value_t value;
  if (dnode_read(node, path, &value) && value.type == DN_ARRAY)
    return value.node;
  return NULL;
}

DataNode dnode_ref_node(DataNode node, slice_t path) {
  dnode_value_t value;
  if (dnode_read(node, path, &value)
  && (value.type == DN_ARRAY || value.type == DN_OBJECT)
  ) {
    return value.node;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

dnode_type_t dnode_get_type(DataNode node, slice_t path) {
  dnode_value_t value;
  if (!dnode_read(node, path, &value)) return DN_NULL;
  return value.type;
}

bool dnode_get_bool(DataNode node, slice_t path) {
  bool ret = false; // default value
  dnode_value_t value;
  if (dnode_read(node, path, &value)) _dnode_coerce_bool(value, &ret);
  return ret;
}

int dnode_get_int(DataNode node, slice_t path) {
  return (int)dnode_get_long(node, path);
}

int64_t dnode_get_long(DataNode node, slice_t path) {
  int64_t ret = 0; // default value
  dnode_value_t value;
  if (dnode_read(node, path, &value)) _dnode_coerce_int(value, &ret);
  return ret;
}

float dnode_get_float(DataNode node, slice_t path) {
  return (float)dnode_get_double(node, path);
}

double dnode_get_double(DataNode node, slice_t path) {
  double ret = 0.0;
  dnode_value_t value;
  if (dnode_read(node, path, &value)) _dnode_coerce_float(value, &ret);
  return ret;
}

slice_t dnode_get_str(DataNode node, slice_t path) {
  slice_t ret = slice_empty;
  dnode_value_t value;
  if (dnode_read(node, path, &value)) _dnode_coerce_string(value, &ret);
  return ret;
}

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

    // Query checking for boolean (or convertable) value
    case DN_BOOL:   _dnode_coerce_bool(value, &query->value_bool);      break;

    // Query checking for integer (or convertable) value
    case DN_INT:    _dnode_coerce_int(value, &query->value_int);        break;

    // Query checking for floating point (or convertable) value
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

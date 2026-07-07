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
#include "data_view.h"
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

static inline bool _valid_node_type(dnode_type_t type) {
  return type >= 0 && type < DN_ARRAY_ELEM_MIXED;
}

////////////////////////////////////////////////////////////////////////////////
// Rules for casting node values between types
////////////////////////////////////////////////////////////////////////////////

bool _dnode_coerce_null(dnode_value_t value, DataNode out) {
  assert(_valid_node_type(value.type));
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

bool _dnode_coerce_bool(dnode_value_t value, bool* out) {
  assert(_valid_node_type(value.type));
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool;               break;
    case DN_INT:    *out = *value.value_int != 0;           break;
    case DN_FLOAT:
      // NaN is false, INF and non-zero is true
      *out = !isnan(*value.value_float) && *value.value_float != 0;
      break;
    case DN_STRING: return slice_to_bool(*value.value_str, out);

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool _dnode_coerce_int(dnode_value_t value, int64_t* out) {
  assert(_valid_node_type(value.type));
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool ? 1 : 0;       break;
    case DN_INT:    *out = *value.value_int;                break;
    case DN_FLOAT:
      // perform no conversion on INF or NaN values (retains defaults)
      if (!isfinite(*value.value_float)) return false;
      *out = (int64_t)*value.value_float;
      break;
    case DN_STRING: return slice_to_long(*value.value_str, out);

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool _dnode_coerce_float(dnode_value_t value, double* out) {
  assert(_valid_node_type(value.type));
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool ? 1 : 0;       break;
    case DN_INT:    *out = (double)*value.value_int;        break;
    case DN_FLOAT:  *out = *value.value_float;              break;
    case DN_STRING: return slice_to_double(*value.value_str, out);

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool _dnode_coerce_string(dnode_value_t value, slice_t* out) {
  assert(_valid_node_type(value.type));
  assert(out);

  switch (value.type) {

    case DN_BOOL:   *out = *value.value_bool ? slice_true : slice_false;  break;
    case DN_STRING: *out = *value.value_str;                              break;

    default: return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool _dnode_coerce_value_node(dnode_value_t value, DataNode out) {
  assert(_valid_node_type(value.type));
  assert(out);

  switch (out->type) {

    case DN_NULL:   return _dnode_coerce_null(value, out);
    case DN_BOOL:   return _dnode_coerce_bool(value, &out->value_bool);
    case DN_INT:    return _dnode_coerce_int(value, &out->value_int);
    case DN_FLOAT:  return _dnode_coerce_float(value, &out->value_float);
    case DN_STRING: return _dnode_coerce_string(value, &out->value_str);

    default: return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Access helpers
////////////////////////////////////////////////////////////////////////////////

 bool _dnode_read_value(DataNode node, dnode_value_t* out) {
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

// Gets a child node of an object based on its name
DataNode _dnode_key_into_object(DataNode node, slice_t key) {
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
bool dview_read_value(DataView _node, slice_t path, dview_value_t* out_value) {
  assert(_node);
  assert(slice_is_valid(path));

  DataNode node = (DataNode)_node;
  dnode_value_t* out = (dnode_value_t*)out_value;

  path = slice_trim(path);

  if (slice_starts_with(path, S("$"))) {
    return _dnode_read_next(node, slice_drop(path, 1), out);
  }

  if (slice_is_empty(path)) {
    return _dnode_read_value(node, out);
  }

  // allow omitting the $ and assume the path starts at an object
  return _dnode_read_member(node, path, out);
}

////////////////////////////////////////////////////////////////////////////////
// Type-specific read accessors with type coercing
////////////////////////////////////////////////////////////////////////////////

bool dview_read_bool(DataView node, slice_t path, bool* out) {
  assert(out);
  dnode_value_t value;
  return dview_read_value(node, path, (void*)&value)
         && _dnode_coerce_bool(value, out);
}

bool dview_read_int(DataView node, slice_t path, int* out) {
  assert(out);
  int64_t res = 0;
  if (!dview_read_long(node, path, &res)) return false;
  *out = (int)res;
  return true;
}

bool dview_read_long(DataView node, slice_t path, int64_t* out) {
  assert(out);
  dnode_value_t value;
  return dview_read_value(node, path, (void*)&value)
         && _dnode_coerce_int(value, out);
}

bool dview_read_float(DataView node, slice_t path, float* out) {
  assert(out);
  double res = 0;
  if (!dview_read_double(node, path, &res)) return false;
  *out = (float)res;
  return true;
}

bool dview_read_double(DataView node, slice_t path, double* out) {
  assert(out);
  dnode_value_t value;
  return dview_read_value(node, path, (void*)&value)
         && _dnode_coerce_float(value, out);
}

bool dview_read_slice(DataView node, slice_t path, slice_t* out) {
  assert(out);
  dnode_value_t value;
  return dview_read_value(node, path, (void*)&value)
         && _dnode_coerce_string(value, out);
}

////////////////////////////////////////////////////////////////////////////////
// Serialization to JSON
////////////////////////////////////////////////////////////////////////////////

void _dnode_to_json(
  DataNode node, Array_byte ret, index_t level, dnode_output_opts_t opts
) {
  bool spacing = !!(opts & OPT_SPACING);

  slice_t obj_memb_kv = S("\":");

  if (spacing) {
    obj_memb_kv = S("\": ");
  }

  switch (node->type) {

    case DN_NULL: {
      arr_byte_append(ret, slice_null);
    } break;

    case DN_BOOL: {
      arr_byte_append(ret, node->value_bool ? slice_true : slice_false);
    } break;

    case DN_INT: {
      arr_byte_append_int(ret, node->value_int);
    } break;

    case DN_FLOAT: {
      arr_byte_append_float(ret, node->value_float, 5);
    } break;

    case DN_STRING: {
      arr_byte_push_back(ret, '"');
      arr_byte_append(ret, node->value_str);
      arr_byte_push_back(ret, '"');
    } break;

    case DN_OBJECT: {
      arr_byte_push_back(ret, '{');
      if (node->object.size > 0) {
        index_t i = 0;
        loop {
          if (spacing) arr_byte_push_back(ret, '\n');
          if (spacing) arr_byte_push_back_repeat(ret, ' ', level * 2 + 2);
          arr_byte_push_back(ret, '"');
          arr_byte_append(ret, node->object.children[i].name);
          arr_byte_append(ret, obj_memb_kv);

          _dnode_to_json(&node->object.children[i].node, ret, level + 1, opts);

          until(++i >= node->object.size);
          arr_byte_push_back(ret, ',');
        }
        if (spacing) arr_byte_push_back(ret, '\n');
        if (spacing) arr_byte_push_back_repeat(ret, ' ', level * 2);
      }
      arr_byte_push_back(ret, '}');
    } break;

    case DN_ARRAY: {
      arr_byte_push_back(ret, '[');
      if (node->array.size > 0) {

        index_t i = 0;
        loop{
          if (spacing) arr_byte_push_back(ret, '\n');
          if (spacing) arr_byte_push_back_repeat(ret, ' ', level * 2 + 2);

          // construct a proxy node to hold the value and recurse one more time
          //    to avoid having to duplicate the per-type printing logic
          DataNode proxy = &(dnode_t) { .type = node->array.elem_type };

          switch (proxy->type) {
            case DN_NULL:   break;
            case DN_BOOL:   proxy->value_bool = node->array.bools[i];     break;
            case DN_INT:    proxy->value_int = node->array.ints[i];       break;
            case DN_FLOAT:  proxy->value_float = node->array.floats[i];   break;
            case DN_STRING: proxy->value_str = node->array.strings[i];    break;

            case DN_OBJECT: SWITCH_FALLTHROUGH;
            case DN_ARRAY:  SWITCH_FALLTHROUGH;
            case DN_ARRAY_ELEM_MIXED:
              proxy = &node->array.nodes[i];
              break;

            default: assert(false); break;
          }

          _dnode_to_json(proxy, ret, level + 1, opts);

          until(++i >= node->array.size);
          arr_byte_push_back(ret, ',');
        }
        if (spacing) arr_byte_push_back(ret, '\n');
        if (spacing) arr_byte_push_back_repeat(ret, ' ', level * 2);
      }
      arr_byte_push_back(ret, ']');
    } break;

    default: assert(false); break;

  }
}

////////////////////////////////////////////////////////////////////////////////

String dview_to_json_opts(DataView node, dnode_output_opts_t opts) {
  array_byte_t _bytes = arr_byte_build_str();
  Array_byte bytes = &_bytes;

  _dnode_to_json((DataNode)node, bytes, 0, opts);
  if (opts & OPT_SPACING) arr_byte_push_back(bytes, '\n');

  return arr_byte_finish_str(bytes);
}

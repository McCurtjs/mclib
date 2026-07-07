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

#ifndef MCLIB_DATA_NODE_H_
#define MCLIB_DATA_NODE_H_

//
// General-Purpose Data Tree Nodes
//
// A node system to represent a tree or sub-tree of a structure which contains
// nodes of generic json-like data with fixed types. This can be used to read or
// write json or to implement logic for other compatible formats.
//
// DataNode trees are mutable, but fixed in shape - they act the same way a
// span_t works with Arrays and view_t. Use DataTree for dynamic data storage.
//
// Data in the tree can be accessed in various ways, either directly with the
// `dnode_ref_...` functions, which return pointers to data only if the type
// matches exactly, or coerced values using `dnode_read...` or `dnode_get...`.
//
// The `path` used for access works as a limited subset of JSONPath:
// - "$"        access the root node (can be any type)
// - "$.blah"   access a member value of a root object
// - "blah"     assume the root node is an object, and access its child "blah"
// - "$[0]"     access an array by index
// - "S['key with spaces']" alternate syntax for accessing object members
//
// Type coercion for the `read` and `get` functions use the following rules:
// - int and float values directly cast between each other
// - bool converts to int/float as 1 or 0, and from int/float as non-0 vs 0
// - string values can be parsed into int, float, or bool
// - numeric data cannot be converted to dynamic Strings (see DataTree)
// - bools convert to/from strings as "true" and "false" (case-insensitive)
//
// Tree data can also be accessed with `dnode_select`, which takes a second
// explicitly mutable tree to match the source data's structure and receive
// the intersecting values. Only data matching a node in the selector tree will
// be copied over from the source, and nodes not present in the source tree will
// keep the default value held by the selector tree. Values copied into the
// selector tree will be type-coerced. A node of type `null` in the selector
// will be used as a wildcard for value nodes and have both its type and value
// updated to match the source tree if a node matches. It will not copy objects
// or arrays, as that would change the structure of the selector tree.
// will be replaced with any matching data leaf node of the source tree.
// (note: string data will not be copied, the resulting tree after selection
// will contain references to data owned by the source tree).
//

#include "types.h"

#include "data_view.h"

//
// Node structure:
//
// Node (typed) --contains--> value --can_be--> [ number ]
//  ^                           ^               [ bool   ]
//  |                           |               [ string ]
//  |                           \----contains-- [ array  ]
//  \------------ key/value pairs <--contains-- [ object ]
//

typedef struct dnode_t dnode_t;
typedef struct dnode_value_t dnode_value_t;
typedef struct dnode_member_t dnode_member_t;
typedef struct dnode_t* DataNode;

// An object node contains only a list of child member values.
typedef struct dnode_object_t {
  union {
    dview_object_t        CONST view;
    struct {
      index_t             CONST size;
      dnode_member_t*     CONST children;
    };
  };
} dnode_object_t;

// An array node can either contain contiguous arrays of plain-data types,
//    or can contain an array of sub-nodes of mixed types, including objects or
//    arrays. In the "mixed" configuration, the types don't all have to match.
typedef struct dnode_array_t {
  union {
    dview_array_t         CONST view;
    struct {
      dnode_type_t        CONST elem_type;
      index_t             CONST size;
      union {
        dnode_t*          CONST nodes;    // - Mixed objects (DN_ARRAY_ELEM_MIXED)
        bool*             CONST bools;    // --/ Homogeneously typed arrays
        int64_t*          CONST ints;     //   |
        double*           CONST floats;   //   |
        slice_t*          CONST strings;  // __/
      };
    };
  };
} dnode_array_t;

// Base data node structure containing either one plain-data value, object
//    dictionary, or array.
struct dnode_t {
  union {
    dview_t               CONST view;
    struct {
      dnode_type_t        CONST type;
      union {
        dnode_array_t           array;
        dnode_object_t    CONST object;
        bool                    value_bool;
        int64_t                 value_int;
        double                  value_float;
        slice_t                 value_str;
      };
    };
  };
};

// A member is only ever contained by an object node, and matches the
//    structure of a regular node but with the addition of name field before
//    the node contents.
typedef struct dnode_member_t {
  union {
    dview_member_t        CONST view;
    struct {
      slice_t             CONST name;
      union {
        dnode_t                 node;
        struct {
          dnode_type_t    CONST type;
          union {
            dnode_object_t      object;
            dnode_array_t       array;
            bool                value_bool;
            int64_t             value_int;
            double              value_float;
            slice_t             value_str;
          };
        };
      };
    };
  };
} dnode_member_t;

// Return type that can give back pointers to values in the structure.
typedef struct dnode_value_t {
  dnode_type_t        CONST type;
  union {
    DataNode          CONST node;
    bool*                   value_bool;
    int64_t*                value_int;
    double*                 value_float;
    slice_t*                value_str;
  };
} dnode_value_t;

////////////////////////////////////////////////////////////////////////////////

DataNode dnode_select(DataNode, DataNode query_and_output);

////////////////////////////////////////////////////////////////////////////////

static inline bool dnode_read_value(
  DataNode node, slice_t path, dnode_value_t* out
) {
  return dview_read_value(&node->view, path, (void*)out);
}

static inline bool dnode_read_bool(DataNode node, slice_t path, bool* out) {
  return dview_read_bool(&node->view, path, out);
}

static inline bool dnode_read_int(DataNode node, slice_t path, int* out) {
  return dview_read_int(&node->view, path, out);
}

static inline bool dnode_read_long(DataNode node, slice_t path, int64_t* out) {
  return dview_read_long(&node->view, path, out);
}

static inline bool dnode_read_float(DataNode node, slice_t path, float* out) {
  return dview_read_float(&node->view, path, out);
}

static inline bool dnode_read_double(DataNode node, slice_t path, double* out) {
  return dview_read_double(&node->view, path, out);
}

static inline bool dnode_read_slice(DataNode node, slice_t path, slice_t* out) {
  return dview_read_slice(&node->view, path, out);
}

static inline bool dnode_contains(DataNode node, slice_t path) {
  return dview_contains(&node->view, path);
}

static inline bool* dnode_ref_bool(DataNode node, slice_t path) {
  return (bool*)dview_ref_bool(&node->view, path);
}

static inline int64_t* dnode_ref_int(DataNode node, slice_t path) {
  return (int64_t*)dview_ref_int(&node->view, path);
}

static inline double* dnode_ref_float(DataNode node, slice_t path) {
  return (double*)dview_ref_float(&node->view, path);
}

static inline slice_t* dnode_ref_str(DataNode node, slice_t path) {
  return (slice_t*)dview_ref_str(&node->view, path);
}

static inline DataNode dnode_ref_object(DataNode node, slice_t path) {
  return (DataNode)dview_ref_object(&node->view, path);
}

static inline DataNode dnode_ref_array(DataNode node, slice_t path) {
  return (DataNode)dview_ref_array(&node->view, path);
}

static inline DataNode dnode_ref_node(DataNode node, slice_t path) {
  return (DataNode)dview_ref_node(&node->view, path);
}

static inline dnode_type_t dnode_get_type(DataNode node, slice_t path) {
  return dview_get_type(&node->view, path);
}

static inline bool dnode_get_or_default_bool(
  DataNode node, slice_t path, bool def
) {
  return dview_get_or_default_bool(&node->view, path, def);
}

static inline int dnode_get_or_default_int(
  DataNode node, slice_t path, int def
) {
  return dview_get_or_default_int(&node->view, path, def);
}

static inline int64_t dnode_get_or_default_long(
  DataNode node, slice_t path, int64_t def
) {
  return dview_get_or_default_long(&node->view, path, def);
}

static inline float dnode_get_or_default_float(
  DataNode node, slice_t path, float def
) {
  return dview_get_or_default_float(&node->view, path, def);
}

static inline double dnode_get_or_default_double(
  DataNode node, slice_t path, double def
) {
  return dview_get_or_default_double(&node->view, path, def);
}

static inline slice_t dnode_get_or_default_str(
  DataNode node, slice_t path, slice_t def
) {
  return dview_get_or_default_str(&node->view, path, def);
}

static inline bool dnode_get_bool(DataNode node, slice_t path) {
  return dnode_get_or_default_bool(node, path, false);
}

static inline int dnode_get_int(DataNode node, slice_t path) {
  return dnode_get_or_default_int(node, path, 0);
}

static inline int64_t dnode_get_long(DataNode node, slice_t path) {
  return dnode_get_or_default_long(node, path, 0);
}

static inline float dnode_get_float(DataNode node, slice_t path) {
  return dnode_get_or_default_float(node, path, 0.0f);
}

static inline double dnode_get_double(DataNode node, slice_t path) {
  return dnode_get_or_default_double(node, path, 0.0);
}

static inline slice_t dnode_get_str(DataNode node, slice_t path) {
  return dnode_get_or_default_str(node, path, slice_empty);
}

static inline String dnode_to_json_opts(
  DataNode node, dnode_output_opts_t opts
) {
  return dview_to_json_opts(&node->view, opts);
}

static inline String dnode_to_json(DataNode node) {
  return dview_to_json(&node->view);
}

static inline String dnode_to_dtn_opts(
  DataNode node, dnode_output_opts_t opts
) {
  return dview_to_dtn_opts(&node->view, opts);
}

static inline String dnode_to_dtn(DataNode node) {
  return dview_to_dtn(&node->view);
}

////////////////////////////////////////////////////////////////////////////////

#define         dnode_read(DNODE, PATH, OUT)                                  \
                  _Generic((OUT),                                             \
                    dnode_value_t*: dnode_read_value,                         \
                    bool*:          dnode_read_bool,                          \
                    int64_t*:       dnode_read_int,                           \
                    double*:        dnode_read_float,                         \
                    slice_t*:       dnode_read_slice                          \
                  )((DNODE), (PATH), (OUT))                                   //

////////////////////////////////////////////////////////////////////////////////

#define         dnode_get_or_default(DNODE, PATH, DEFAULT_VALUE)              \
                  _Generic((DEFAULT_VALUE),                                   \
                    bool:     dnode_get_or_default_bool,                      \
                    int:      dnode_get_or_default_int,                       \
                    int64_t:  dnode_get_or_default_long,                      \
                    float:    dnode_get_or_default_float,                     \
                    double:   dnode_get_or_default_double,                    \
                    slice_t:  dnode_get_or_default_str                        \
                  )((DNODE), (PATH), (DEFAULT_VALUE))                         //

////////////////////////////////////////////////////////////////////////////////

#ifndef _s2r
# define _s2r(X) X
#endif

#define NODE_ROOT(ROOT_NODE) &(ROOT_NODE)
#define VIEW_ROOT(ROOT_NODE) &(ROOT_NODE).view

#define NODE_NULL ((dnode_t) { .type = DN_NULL })
#define MEMB_NULL(NAME) ((dnode_member_t) {                                   \
  .name = _s2r(NAME), .type = DN_NULL                                         \
})                                                                            //

#define NODE_BOOL(VALUE) ((dnode_t) { .type = DN_BOOL, .value_bool = (VALUE) })
#define MEMB_BOOL(NAME, VALUE) ((dnode_member_t) {                            \
  .name = _s2r(NAME), .type = DN_BOOL, .value_bool = (VALUE)                  \
})                                                                            //

#define NODE_INT(VALUE) ((dnode_t) { .type = DN_INT, .value_int = (VALUE) })
#define MEMB_INT(NAME, VALUE) ((dnode_member_t) {                             \
  .name = _s2r(NAME), .type = DN_INT, .value_int = (VALUE)                    \
})                                                                            //

#define NODE_FLOAT(VALUE) ((dnode_t) {                                        \
.type = DN_FLOAT, .value_float = (VALUE)                                      \
})                                                                            //
#define MEMB_FLOAT(NAME, VALUE) ((dnode_member_t) {                           \
  .name = _s2r(NAME), .type = DN_FLOAT, .value_float = (VALUE)                \
})                                                                            //

#define NODE_STRING(VALUE) ((dnode_t) {                                       \
  .type = DN_STRING, .value_str = _s2r(VALUE)                                 \
})                                                                            //
#define MEMB_STRING(NAME, VALUE) ((dnode_member_t) {                          \
  .name = _s2r(NAME), .type = DN_STRING, .value_str = _s2r(VALUE)             \
})                                                                            //

#define NODE_OBJECT_EMPTY ((dnode_t) { .type = DN_OBJECT, .object.size = 0 })
#define MEMB_OBJECT_EMPTY(NAME) ((dnode_member_t) {                           \
  .name = _s2r(NAME), .type = DN_OBJECT, .object.size = 0                     \
})                                                                            //

#define NODE_OBJECT(...) ((dnode_t) {                                         \
  .type = DN_OBJECT, .object.size = _va_count(__VA_ARGS__),                   \
  .object.children = ((dnode_member_t[_va_count(__VA_ARGS__)]) {__VA_ARGS__}) \
})                                                                            //
#define MEMB_OBJECT(NAME, ...) ((dnode_member_t) { .name = _s2r(NAME),        \
  .type = DN_OBJECT, .object.size = _va_count(__VA_ARGS__),                   \
  .object.children = ((dnode_member_t[_va_count(__VA_ARGS__)]) {__VA_ARGS__}) \
})                                                                            //

#define _NODE_ARRAY_INNER(DN_TYPE, TYPE, MEMBER, ...)                         \
  .type = DN_ARRAY, .array.elem_type = DN_TYPE,                               \
  .array.size = _va_count(__VA_ARGS__),                                       \
  .array.MEMBER = ((TYPE[_va_count(__VA_ARGS__)]) { __VA_ARGS__ })            //

#define NODE_ARRAY_EMPTY ((dnode_t) {                                         \
  .type = DN_ARRAY, .array.elem_type = DN_NULL,                               \
  .array.size = 0, .array.nodes = NULL                                        \
})                                                                            //

#define MEMB_ARRAY_EMPTY(NAME) ((dnode_member_t) {                            \
  .name = _s2r(NAME), .type = DN_ARRAY, .array.elem_type = DN_NULL,           \
  .array.size = 0, .array.nodes = NULL                                        \
})                                                                            //

#define NODE_ARRAY(...) ((dnode_t) {                                          \
  _NODE_ARRAY_INNER(DN_ARRAY_ELEM_MIXED, dnode_t, nodes, __VA_ARGS__)         \
})                                                                            //
#define MEMB_ARRAY(NAME, ...) ((dnode_member_t) { .name = (NAME),             \
  _NODE_ARRAY_INNER(DN_ARRAY_ELEM_MIXED, dnode_t, nodes, __VA_ARGS__)         \
})                                                                            //

#define NODE_ARRAY_OBJECT(...) ((dnode_t) {                                   \
  _NODE_ARRAY_INNER(DN_OBJECT, dnode_t, nodes, __VA_ARGS__)                   \
})                                                                            //
#define MEMB_ARRAY_OBJECT(NAME, ...) ((dnode_member_t) { .name = _s2r(NAME),  \
  _NODE_ARRAY_INNER(DN_OBJECT, dnode_t, nodes, __VA_ARGS__)                   \
})                                                                            //

#define NODE_ARRAY_ARRAY(...) ((dnode_t) {                                    \
  _NODE_ARRAY_INNER(DN_ARRAY, dnode_t, nodes, __VA_ARGS__)                    \
})                                                                            //
#define MEMB_ARRAY_ARRAY(NAME, ...) ((dnode_member_t) { .name = _s2r(NAME),   \
  _NODE_ARRAY_INNER(DN_ARRAY, dnode_t, nodes, __VA_ARGS__)                    \
})                                                                            //

#define NODE_ARRAY_BOOL(...) ((dnode_t) {                                     \
  _NODE_ARRAY_INNER(DN_BOOL, bool, bools, __VA_ARGS__)                        \
})                                                                            //
#define MEMB_ARRAY_BOOL(NAME, ...) ((dnode_member_t) { .name = _s2r(NAME),    \
  _NODE_ARRAY_INNER(DN_BOOL, bool, bools, __VA_ARGS__)                        \
})                                                                            //

#define NODE_ARRAY_INT(...) ((dnode_t) {                                      \
  _NODE_ARRAY_INNER(DN_INT, int64_t, ints, __VA_ARGS__)                       \
})                                                                            //
#define MEMB_ARRAY_INT(NAME, ...) ((dnode_member_t) { .name = _s2r(NAME),     \
  _NODE_ARRAY_INNER(DN_INT, int64_t, ints, __VA_ARGS__)                       \
})                                                                            //

#define NODE_ARRAY_FLOAT(...) ((dnode_t) {                                    \
  _NODE_ARRAY_INNER(DN_FLOAT, double, floats, __VA_ARGS__)                    \
})                                                                            //
#define MEMB_ARRAY_FLOAT(NAME, ...) ((dnode_member_t) { .name = _s2r(NAME),   \
  _NODE_ARRAY_INNER(DN_FLOAT, double, floats, __VA_ARGS__)                    \
})                                                                            //

#define NODE_ARRAY_STRING(...) ((dnode_t) {                                   \
  _NODE_ARRAY_INNER(DN_STRING, slice_t, strings, __VA_ARGS__)                 \
})                                                                            //
#define MEMB_ARRAY_STRING(NAME, ...) ((dnode_member_t) { .name = _s2r(NAME),  \
  _NODE_ARRAY_INNER(DN_STRING, slice_t, strings, __VA_ARGS__                  \
})                                                                            //

#endif

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

#ifndef MCLIB_DATA_VIEW_H_
#define MCLIB_DATA_VIEW_H_

//
// General-Purpose Data Tree View
//
// A read-only node system to represent a tree or sub-tree of DataNodes, which
// form a generic tree structure with fixed typing that can be arranged in
// arrays and dictionaries to mimic the storage patterns of json data.
//
// DataView trees are immutable and fixed in shape - they act the same way a
// view_t works with Arrays and span_t. Use DataTree for dynamic data storage.
//
// Data in the tree can be accessed in various ways, either directly with the
// `dview_ref_...` functions, which return pointers to data only if the type
// matches exactly, or coerced values using `dview_read...` or `dview_get...`.
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
// - string values can be parsed into int, float, or bool (not vice-versa)
// - bools convert to/from strings as "true" and "false" (case-insensitive)
//

#include "types.h"
#include "slice.h"

// Node structure:
//
// Node (typed) --contains--> value --can_be--> [ number ]
//  ^                           ^               [ bool   ]
//  |                           |               [ string ]
//  |                           \----contains-- [ array  ]
//  \------------ key/value pairs <--contains-- [ object ]
//

typedef enum {
  DN_NULL,
  DN_OBJECT,
  DN_ARRAY,
  DN_BOOL,
  DN_INT,
  DN_FLOAT,
  DN_STRING,
//DN_VEC2,
//DN_VEC3,
//DN_VEC4,
//DN_VEC2I,
//DN_VEC3I,
//DN_VEC3B,
//DN_VEC4B,
//DN_MAT3,
//DN_MAT4,
  DN_ARRAY_ELEM_MIXED
} dnode_type_t;

typedef enum {
  OPT_DEFAULT = 0x00,
  OPT_SPACING = 0x01
} dnode_output_opts_t;

typedef struct dview_t dview_t;
typedef struct dview_value_t dview_value_t;
typedef struct dview_member_t dview_member_t;
typedef struct dview_t  const* DataView;

typedef struct dview_object_t {
  index_t               CONST size;
  dview_member_t*       CONST children;
} dview_object_t;

typedef struct dview_array_t {
  dnode_type_t          CONST elem_type;
  index_t               CONST size;
  union {
    CONST dview_t*      CONST nodes;
    CONST bool*         CONST bools;
    CONST int64_t*      CONST ints;
    CONST double*       CONST floats;
    CONST slice_t*      CONST strings;
  };
} dview_array_t;

struct dview_t {
  dnode_type_t          CONST type;
  union {
    dview_array_t       CONST array;
    dview_object_t      CONST object;
    bool                CONST value_bool;
    int64_t             CONST value_int;
    double              CONST value_float;
    slice_t             CONST value_str;
  };
};

typedef struct dview_member_t {
  slice_t               CONST name;
  union {
    dview_t             CONST node;
    struct {
      dnode_type_t      CONST type;
      union {
        dview_object_t  CONST object;
        dview_array_t   CONST array;
        bool            CONST value_bool;
        int64_t         CONST value_int;
        double          CONST value_float;
        slice_t         CONST value_str;
      };
    };
  };
} dview_member_t;

typedef struct dview_value_t {
  dnode_type_t          CONST type;
  union {
    DataView            CONST node;
    CONST bool*         CONST value_bool;
    CONST int64_t*      CONST value_int;
    CONST double*       CONST value_float;
    CONST slice_t*      CONST value_str;
  };
} dview_value_t;

bool    dview_read_value(DataView, slice_t path, dview_value_t* out);

String  dview_to_json_opts(DataView, dnode_output_opts_t);
static inline String dview_to_json(DataView);

String  dview_to_dtn_opts(DataView, dnode_output_opts_t);
static inline String dview_to_dtn(DataView);

bool    dview_read_bool(DataView, slice_t path, bool* out);
bool    dview_read_int(DataView, slice_t path, int* out);
bool    dview_read_long(DataView, slice_t path, int64_t* out);
bool    dview_read_float(DataView, slice_t path, float* out);
bool    dview_read_double(DataView, slice_t path, double* out);
bool    dview_read_slice(DataView, slice_t path, slice_t* out);

static inline bool dview_contains(DataView node, slice_t path) {
  return dview_read_value(node, path, NULL);
}

static inline const bool* dview_ref_bool(DataView node, slice_t path) {
  dview_value_t value;
  if (dview_read_value(node, path, &value) && value.type == DN_BOOL)
    return value.value_bool;
  return NULL;
}

static inline const int64_t* dview_ref_int(DataView node, slice_t path) {
  dview_value_t value;
  if (dview_read_value(node, path, &value) && value.type == DN_INT)
    return value.value_int;
  return NULL;
}

static inline const double* dview_ref_float(DataView node, slice_t path) {
  dview_value_t value;
  if (dview_read_value(node, path, &value) && value.type == DN_FLOAT)
    return value.value_float;
  return NULL;
}

static inline const slice_t* dview_ref_str(DataView node, slice_t path) {
  dview_value_t value;
  if (dview_read_value(node, path, &value) && value.type == DN_STRING)
    return value.value_str;
  return NULL;
}

static inline DataView dview_ref_object(DataView node, slice_t path) {
  dview_value_t value;
  if (dview_read_value(node, path, &value) && value.type == DN_OBJECT)
    return value.node;
  return NULL;
}

static inline DataView dview_ref_array(DataView node, slice_t path) {
  dview_value_t value;
  if (dview_read_value(node, path, &value) && value.type == DN_ARRAY)
    return value.node;
  return NULL;
}

static inline DataView dview_ref_node(DataView node, slice_t path) {
  dview_value_t value;
  if (dview_read_value(node, path, &value)
  && (value.type == DN_ARRAY || value.type == DN_OBJECT)
  ) {
    return value.node;
  }
  return NULL;
}

static inline bool dview_get_or_default_bool(
  DataView data, slice_t path, bool def
) {
  bool res;
  if (dview_read_bool(data, path, &res)) return res;
  return def;
}

static inline int dview_get_or_default_int(
  DataView data, slice_t path, int def
) {
  int res;
  if (dview_read_int(data, path, &res)) return res;
  return def;
}

static inline int64_t dview_get_or_default_long(
  DataView data, slice_t path, int64_t def
) {
  int64_t res;
  if (dview_read_long(data, path, &res)) return res;
  return def;
}

static inline float dview_get_or_default_float(
  DataView data, slice_t path, float def
) {
  float res;
  if (dview_read_float(data, path, &res)) return res;
  return def;
}

static inline double dview_get_or_default_double(
  DataView data, slice_t path, double def
) {
  double res;
  if (dview_read_double(data, path, &res)) return res;
  return def;
}

static inline slice_t dview_get_or_default_str(
  DataView data, slice_t path, slice_t def
) {
  slice_t res;
  if (dview_read_slice(data, path, &res)) return res;
  return def;
}

static inline dnode_type_t dview_get_type(DataView node, slice_t path) {
  dview_value_t value;
  if (!dview_read_value(node, path, &value)) return DN_NULL;
  return value.type;
}

static inline bool dview_get_bool(DataView node, slice_t path) {
  return dview_get_or_default_bool(node, path, false);
}

static inline int dview_get_int(DataView node, slice_t path) {
  return dview_get_or_default_int(node, path, 0);
}

static inline int64_t dview_get_long(DataView node, slice_t path) {
  return dview_get_or_default_long(node, path, 0);
}

static inline float dview_get_float(DataView node, slice_t path) {
  return dview_get_or_default_float(node, path, 0.f);
}

static inline double dview_get_double(DataView node, slice_t path) {
  return dview_get_or_default_double(node, path, 0.0);
}

static inline slice_t dview_get_str(DataView node, slice_t path) {
  return dview_get_or_default_str(node, path, slice_empty);
}

static inline String dview_to_json(DataView data) {
  return dview_to_json_opts(data, OPT_DEFAULT);
}

static inline String dview_to_dtn(DataView data) {
  return dview_to_dtn_opts(data, OPT_DEFAULT);
}

////////////////////////////////////////////////////////////////////////////////

#define         dview_read(DNODE, PATH, OUT)                                  \
                  _Generic((OUT),                                             \
                    dview_value_t*: dview_read_value,                         \
                    bool*:          dview_read_bool,                          \
                    int64_t*:       dview_read_int,                           \
                    double*:        dview_read_float,                         \
                    slice_t*:       dview_read_slice,                         \
                  )((DNODE), (PATH), (OUT))                                   //

////////////////////////////////////////////////////////////////////////////////

#define         dview_get_or_default(DNODE, PATH, DEFAULT_VALUE)              \
                  _Generic((DEFAULT_VALUE),                                   \
                    bool:           dview_get_or_default_bool,                \
                    int:            dview_get_or_default_int,                 \
                    int64_t:        dview_get_or_default_long,                \
                    float:          dview_get_or_default_float,               \
                    double:         dview_get_or_default_double,              \
                    slice_t:        dview_get_or_default_str,                 \
                  )((DNODE), (PATH), (DEFAULT_VALUE))                         //

////////////////////////////////////////////////////////////////////////////////

#endif

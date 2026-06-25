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

#include "types.h"

#include "slice.h"
#include "span.h"
#include "string.h"

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
  DN_ARRAY_ELEM_MIXED
} dnode_type_t;

typedef struct dnode_t dnode_t;
typedef struct dnode_value_t dnode_value_t;
typedef struct dnode_member_t dnode_member_t;
typedef struct dnode_t* DataNode;

typedef struct dnode_object_t {
  index_t             CONST size;
  dnode_member_t*     CONST children;
}*DataNode_Object, dnode_object_t;

typedef struct dnode_array_t {
  dnode_type_t        CONST elem_type;
  index_t             CONST size;
  union {
    dnode_t*          CONST nodes;    // - Mixed objects (DN_ARRAY_ELEM_MIXED)
    bool*             CONST bools;    // --/ Homogeneously typed arrays
    int64_t*          CONST ints;     //   |
    double*           CONST floats;   //   |
    slice_t*          CONST strings;  // __/
  };
}*DataNode_Array, dnode_array_t;

// Data Node
struct dnode_t {
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

typedef struct dnode_member_t {
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
} dnode_member_t;

// Return type that can give back pointers to values in the structure
typedef struct dnode_value_t {
  dnode_type_t        CONST type;
  union {
    DataNode          CONST node;
    bool* value_bool;
    int64_t* value_int;
    double* value_float;
    slice_t* value_str;
  };
} dnode_value_t;

bool      dnode_read(DataNode, slice_t path, dnode_value_t* out_value);
bool      dnode_contains(const DataNode, slice_t path);
bool*     dnode_ref_bool(DataNode, slice_t path);
int64_t*  dnode_ref_int(DataNode, slice_t path);
double*   dnode_ref_float(DataNode, slice_t path);
slice_t*  dnode_ref_str(DataNode, slice_t path);
DataNode  dnode_ref_object(DataNode, slice_t path);
bool      dnode_get_bool(const DataNode, slice_t path);
int       dnode_get_int(const DataNode, slice_t path);
int64_t   dnode_get_long(const DataNode, slice_t path);
float     dnode_get_float(const DataNode, slice_t path);
double    dnode_get_double(const DataNode, slice_t path);
slice_t   dnode_get_str(const DataNode, slice_t path);

DataNode  dnode_select(const DataNode input, DataNode query_and_output);

String    dnode_to_json(DataNode node);

////////////////////////////////////////////////////////////////////////////////

#define NODE_ROOT(ROOT_NODE) &(ROOT_NODE)

#define NODE_NULL ((dnode_t) { .type = DN_NULL })
#define MEMB_NULL(NAME) ((dnode_member_t) { .name = (NAME), .type = DN_NULL })

#define NODE_BOOL(VALUE) ((dnode_t) { .type = DN_BOOL, .value_bool = (VALUE) })
#define MEMB_BOOL(NAME, VALUE) ((dnode_member_t) {                            \
  .name = (NAME), .type = DN_BOOL, .value_bool = (VALUE)                      \
})                                                                            //

#define NODE_INT(VALUE) ((dnode_t) { .type = DN_INT, .value_int = (VALUE) })
#define MEMB_INT(NAME, VALUE) ((dnode_member_t) {                             \
  .name = (NAME), .type = DN_INT, .value_int = (VALUE)                        \
})                                                                            //

#define NODE_FLOAT(VALUE) ((dnode_t) {                                        \
.type = DN_FLOAT, .value_float = (VALUE)                                      \
})                                                                            //
#define MEMB_FLOAT(NAME, VALUE) ((dnode_member_t) {                           \
  .name = (NAME), .type = DN_FLOAT, .value_float = (VALUE)                    \
})                                                                            //

#define NODE_STRING(VALUE) ((dnode_t) {                                       \
  .type = DN_STRING, .value_str = (VALUE)                                     \
})                                                                            //
#define MEMB_STRING(NAME, VALUE) ((dnode_member_t) {                          \
  .name = (NAME), .type = DN_STRING, .value_str = (VALUE)                     \
})                                                                            //

#define NODE_OBJECT_EMPTY ((dnode_t) { .type = DN_OBJECT, .object.size = 0 })
#define MEMB_OBJECT_EMPTY(NAME) ((dnode_member_t) {                           \
  .name = (NAME), .type = DN_OBJECT, .object.size = 0                         \
})                                                                            //

#define NODE_OBJECT(...) ((dnode_t) {                                         \
  .type = DN_OBJECT, .object.size = _va_count(__VA_ARGS__),                   \
  .object.children = ((dnode_member_t[_va_count(__VA_ARGS__)]) {__VA_ARGS__}) \
})                                                                            //
#define MEMB_OBJECT(NAME, ...) ((dnode_member_t) {                            \
  .name = (NAME), .type = DN_OBJECT, .object.size = _va_count(__VA_ARGS__),   \
  .object.children = ((dnode_member_t[_va_count(__VA_ARGS__)]) {__VA_ARGS__}) \
})                                                                            //

#define _NODE_ARRAY_INNER(DN_TYPE, TYPE, MEMBER, ...)                         \
  .type = DN_ARRAY, .array.elem_type = DN_TYPE,                               \
  .array.size = _va_count(__VA_ARGS__),                                       \
  .array.MEMBER = ((TYPE[_va_count(__VA_ARGS__)]) { __VA_ARGS__ })            //

#define NODE_ARRAY(...) ((dnode_t) {                                          \
  _NODE_ARRAY_INNER(DN_ARRAY_ELEM_MIXED, dnode_t, nodes, __VA_ARGS__)         \
})                                                                            //
#define MEMB_ARRAY(NAME, ...) ((dnode_member_t) { .name = (NAME),             \
  _NODE_ARRAY_INNER(DN_ARRAY_ELEM_MIXED, dnode_t, nodes, __VA_ARGS__)         \
})                                                                            //

#define NODE_ARRAY_OBJECT(...) ((dnode_t) {                                   \
  _NODE_ARRAY_INNER(DN_OBJECT, dnode_t, nodes, __VA_ARGS__)                   \
})                                                                            //
#define MEMB_ARRAY_OBJECT(NAME, ...) ((dnode_member_t) { .name = (NAME),      \
  _NODE_ARRAY_INNER(DN_OBJECT, dnode_t, nodes, __VA_ARGS__)                   \
})                                                                            //

#define NODE_ARRAY_ARRAY(...) ((dnode_t) {                                    \
  _NODE_ARRAY_INNER(DN_ARRAY, dnode_t, nodes, __VA_ARGS__)                    \
})                                                                            //
#define MEMB_ARRAY_ARRAY(NAME, ...) ((dnode_member_t) { .name = (NAME),       \
  _NODE_ARRAY_INNER(DN_ARRAY, dnode_t, nodes, __VA_ARGS__)                    \
})                                                                            //

#define NODE_ARRAY_BOOL(...) ((dnode_t) {                                     \
  _NODE_ARRAY_INNER(DN_BOOL, bool, bools, __VA_ARGS__)                        \
})                                                                            //
#define MEMB_ARRAY_BOOL(NAME, ...) ((dnode_member_t) { .name = (NAME),        \
  _NODE_ARRAY_INNER(DN_BOOL, bool, bools, __VA_ARGS__)                        \
})                                                                            //

#define NODE_ARRAY_INT(...) ((dnode_t) {                                      \
  _NODE_ARRAY_INNER(DN_INT, int64_t, ints, __VA_ARGS__)                       \
})                                                                            //
#define MEMB_ARRAY_INT(NAME, ...) ((dnode_member_t) { .name = (NAME),         \
  _NODE_ARRAY_INNER(DN_INT, int64_t, ints, __VA_ARGS__)                       \
})                                                                            //

#define NODE_ARRAY_FLOAT(...) ((dnode_t) {                                    \
  _NODE_ARRAY_INNER(DN_FLOAT, double, floats, __VA_ARGS__)                    \
})                                                                            //
#define MEMB_ARRAY_FLOAT(NAME, ...) ((dnode_member_t) { .name = (NAME),       \
  _NODE_ARRAY_INNER(DN_FLOAT, double, floats, __VA_ARGS__)                    \
})                                                                            //

#define NODE_ARRAY_STRING(...) ((dnode_t) {                                   \
  _NODE_ARRAY_INNER(DN_STRING, slice_t, strings, __VA_ARGS__)                 \
})                                                                            //
#define MEMB_ARRAY_STRING(NAME, ...) ((dnode_member_t) { .name = (NAME),      \
  _NODE_ARRAY_INNER(DN_STRING, slice_t, strings, __VA_ARGS__                  \
})                                                                            //

#endif

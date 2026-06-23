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
int64_t   dnode_get_int(DataNode, slice_t path);
double    dnode_get_float(DataNode, slice_t path);
DataNode  dnode_select(const DataNode input, DataNode query_and_output);

String    dnode_to_json(DataNode node);

////////////////////////////////////////////////////////////////////////////////



#endif

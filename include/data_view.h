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
  DN_ARRAY_ELEM_MIXED
} dnode_type_t;

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

bool            dview_read(DataView, slice_t path, dview_value_t* out_value);
bool            dview_read_bool(DataView, slice_t path, bool* out);
bool            dview_read_int(DataView, slice_t path, int* out);
bool            dview_read_long(DataView, slice_t path, int64_t* out);
bool            dview_read_float(DataView, slice_t path, float* out);
bool            dview_read_double(DataView, slice_t path, double* out);
bool            dview_read_slice(DataView, slice_t path, slice_t* out);
bool            dview_contains(DataView, slice_t path);
const bool*     dview_ref_bool(DataView, slice_t path);
const int64_t*  dview_ref_int(DataView, slice_t path);
const double*   dview_ref_float(DataView, slice_t path);
const slice_t*  dview_ref_str(DataView, slice_t path);
DataView        dview_ref_object(DataView, slice_t path);
DataView        dview_ref_array(DataView, slice_t path);
DataView        dview_ref_node(DataView, slice_t path);
dnode_type_t    dview_get_type(DataView, slice_t path);
bool            dview_get_bool(DataView, slice_t path);
int             dview_get_int(DataView, slice_t path);
int64_t         dview_get_long(DataView, slice_t path);
float           dview_get_float(DataView, slice_t path);
double          dview_get_double(DataView, slice_t path);
slice_t         dview_get_str(DataView, slice_t path);
bool            dview_get_or_default_bool(DataView, slice_t path, bool def);
int             dview_get_or_default_int(DataView, slice_t path, int def);
int64_t         dview_get_or_default_long(DataView, slice_t path, int64_t def);
float           dview_get_or_default_float(DataView, slice_t path, float def);
double          dview_get_or_default_double(DataView, slice_t path, double def);
slice_t         dview_get_or_default_str(DataView, slice_t path, slice_t def);

DataView        dview_select(DataView input, DataView query_and_output);

#endif

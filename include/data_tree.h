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

#ifndef MCLIB_DATA_TREE_H_
#define MCLIB_DATA_TREE_H_

//
// General-Purpose Dynamic Data Tree Container
//
// A data tree system representing a tree containing general-purpose fixed-type
// nodes of data that map cleanly to json-like structures. This can be used to
// read or write json or to implement logic for similar compatible formats.
//

//
// Node structure:
//
// Node (typed) --contains--> value --can_be--> [ number ]
//  ^                           ^               [ bool   ]
//  |                           |               [ string ]
//  |                           \----contains-- [ array  ]
//  \------------ key/value pairs <--contains-- [ object ]
//

#include "types.h"

#include "data_node.h"
#include "view_byte.h"

typedef enum dtree_status_t {
  DS_READY,
  DS_STR_EOF,
  DS_VAL_PARSE_ERROR,
  DS_VAL_JUNK_BEFORE_OBJ,
  DS_VAL_JUNK_BEFORE_ARR,
  DS_VAL_JUNK_BEFORE_STR,
  DS_VAL_JUNK_AFTER_OBJ,
  DS_VAL_JUNK_AFTER_ARR,
  DS_VAL_JUNK_AFTER_STR,
  DS_OBJ_EOF,
  DS_OBJ_EOF_AFTER_KEY,
  DS_OBJ_GARBAGE_BEFORE_MEMBER_NAME,
  DS_OBJ_GARBAGE_AFTER_MEMBER_NAME,
  DS_OBJ_INVALID_SEPARATOR,
  DS_ARR_EOF,
  DS_ARR_EMPTY_VALUE
} dtree_status_t;

// While DataNode can represent an entire structure, it does not own the data it
//    contains. A DataTree both contains and owns a copy of its data.
typedef struct _opaque_DataTree_t {
  union {
    DataNode            CONST root;
    DataView            CONST view;
  };
  dtree_status_t        CONST status;
  index_t               CONST error_pos;
}* DataTree;

DataTree  dtree_copy(DataView tree_to_copy);
DataTree  dtree_from_json(slice_t json_string);
DataTree  dtree_from_dtn(slice_t dtn_string);
DataTree  dtree_from_dtb(view_byte_t dtb_data);

DataTree  dtree_copy_select(DataView source, DataView query);
DataNode  dtree_select(DataNode source, DataNode query_and_output);

void      dtree_delete(DataTree* to_delete);

DataNode  dtree_add_member(DataTree, slice_t parent_path, dnode_member_t);
DataNode  dtree_add_node(DataTree, slice_t parent_path, dnode_t);
DataNode  dtree_add_value(DataTree, slice_t path, dnode_value_t);

#endif

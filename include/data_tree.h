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

// While DataNode can represent an entire structure, it does not own the data it
//    contains. A DataTree both contains and owns a copy of its data.
typedef struct _opaque_DataTree_t {
  DataNode            CONST root;
}* DataTree;

DataTree  dtree_new(dnode_type_t root_node_type);
DataTree  dtree_compress(DataTree tree);
void      dtree_delete(DataTree* to_delete);

DataNode  dtree_replace(DataNode node, dnode_type_t type);

#endif

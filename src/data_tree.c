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
#include "data_tree.h"

#include "array_byte.h"

#include <stdlib.h>

// Disable annoying warnings in test when assert is replaced with cspec_assert.
//    these warnings appear because intellisense doesn't recognize that
//    cspec_assert blocks further execution.
#if defined(MCLIB_TEST_MODE) && defined(_MSC_VER)
# pragma warning ( disable : 6011 )
# pragma warning ( disable : 6387 )
#endif

#define con_type struct dnode_t
#define con_prefix dnode
#include "array.h"
#undef con_prefix
#undef con_type

typedef struct DataTree_Internal {
  struct _opaque_DataTree_t pub;

  array_dnode_t nodes;
  array_byte_t  data;
} DataTree_Internal;

DataTree dtree_new(dnode_type_t root_node_type) {
  DataTree_Internal* ret = calloc(1, (sizeof(*ret)));
  assert(ret);

  arr_dnode_init_reserve(&ret->nodes, 10);
  arr_byte_init(&ret->data);
  ret->pub.root = NULL;

  return (DataTree)ret;
}

void dtree_delete(DataTree* p_dtree) {
  if (!p_dtree || !*p_dtree) return;
  DataTree_Internal* dtree = (DataTree_Internal*)*p_dtree;
  Array_dnode nodes = &dtree->nodes;
  Array_byte data = &dtree->data;
  arr_dnode_delete(&nodes);
  arr_byte_delete(&data);
  *p_dtree = NULL;
}

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

#include "data_tree.h"

#include "str.h"

#define CSPEC_CUSTOM_TYPES \
  slice_t: "slice_t",       \
  DataTree: "dtree_t*", DataNode: "dnode_t*", default: "void*"

#include "cspec.h"

describe(dtree_copy) {

  DataTree dtree = NULL;

  it("Should create a tree with a single node and delete successfully") {
    dtree = dtree_copy(VIEW_ROOT(NODE_NULL));
  }

  it("should copy a tree with a limited set of items") {
    dtree = dtree_copy(VIEW_ROOT(
      NODE_OBJECT(
        MEMB_INT("herp", 5),
        MEMB_STRING("derp", "5"),
        MEMB_OBJECT("test",
          MEMB_INT("sub", 6),
          MEMB_NULL("nothin")
        ),
        MEMB_ARRAY(S("arr"),
          NODE_BOOL(false),
          NODE_INT(5),
          NODE_STRING("string"),
          NODE_OBJECT_EMPTY,
          NODE_ARRAY_FLOAT(1, 2, 3.087, 4, 5),
          NODE_ARRAY_EMPTY
        )
      )
    ));
    String xyz = dnode_to_json_opts(dtree->root, OPT_SPACING);
    str_delete(&xyz);
  }

  after {
    dtree_delete(&dtree);
    expect(dtree to be_null);
  }

}

describe(dtree_from_json) {

  DataTree dtree = NULL;

  it("can process a json string with an empty object") {
    dtree = dtree_from_json(S("{}"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_OBJECT);
    expect(dtree->root->object.size, == , 0);
    expect(dtree->root->object.children to be_null);
  }

  it("can process a json string value into a DataTree string node") {
    dtree = dtree_from_json(S("\"a string\""));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_STRING);
    expect(slice_eq to be_true given(dtree->root->value_str, S("a string")));
  }

  it("can process a json string value with regular escaped characters") {
    dtree = dtree_from_json(S("\"string \\\" escape \\\\ chars\""));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_STRING);
    slice_t result = dtree->root->value_str;
    expect(slice_eq to be_true given(result, S("string \" escape \\ chars")));
  }

  it("can process a boolean string into a DataNode") {
    dtree = dtree_from_json(S("true"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_BOOL);
    expect(dtree->root->value_bool to be_true);
  }

  it("can process a json null value into a DataTree string node") {
    dtree = dtree_from_json(S("null"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_NULL);
  }

  it("can process a number into a DataNode float") {
    dtree = dtree_from_json(S("-123.45"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_FLOAT);
    expect(dtree->root->value_float to be_about(-123.45));
  }

  it("can process a number into a DataNode int") {
    dtree = dtree_from_json(S("-123"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_INT);
    expect(dtree->root->value_int, == , -123);
  }

  it("can process members of an object with basic types") {
    dtree = dtree_from_json(
      S("{\"i\":5, \"f\" : 12.3 , \"b\"\t: false, \"t\":true , \" n \" : null}")
    );
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_OBJECT);
    expect(dtree->root->object.size, == , 5);
    expect(dtree->root->object.children[0].type, == , DN_INT);
    expect(dtree->root->object.children[1].type, == , DN_FLOAT);
    expect(dtree->root->object.children[2].type, == , DN_BOOL);
    expect(dtree->root->object.children[3].type, == , DN_BOOL);
    expect(dtree->root->object.children[4].type, == , DN_NULL);
    expect(dtree->root->object.children[0].value_int, == , 5);
    expect(dtree->root->object.children[1].value_float to be_about(12.3));
    expect(dtree->root->object.children[2].value_bool, == , false);
    expect(dtree->root->object.children[3].value_bool, == , true);
    dnode_member_t* children = dtree->root->object.children;
    expect(slice_eq to be_true given(children[0].name, S("i")));
    expect(slice_eq to be_true given(children[1].name, S("f")));
    expect(slice_eq to be_true given(children[2].name, S("b")));
    expect(slice_eq to be_true given(children[3].name, S("t")));
    expect(slice_eq to be_true given(children[4].name, S(" n ")));
  }

  it("can process string members of an object") {
    dtree = dtree_from_json(S("{ \"key\":\"value\", \"second\" :  \"stuff\"}"));
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_OBJECT);
    expect(dtree->root->object.size, == , 2);
    expect(dtree->root->object.children[0].type, == , DN_STRING);
    expect(dtree->root->object.children[1].type, == , DN_STRING);
    dnode_member_t* children = dtree->root->object.children;
    expect(slice_eq to be_true given(children[0].value_str, S("value")));
    expect(slice_eq to be_true given(children[1].value_str, S("stuff")));
    expect(slice_eq to be_true given(children[0].name, S("key")));
    expect(slice_eq to be_true given(children[1].name, S("second")));
  }

  it("can recursively read objects") {
    dtree = dtree_from_json(S("{ \"sub\":{\"item\":\"value\"}}"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_OBJECT);
    expect(dtree->root->object.size, == , 1);
    dnode_member_t* child = dtree->root->object.children;
    expect(child to not be_null);
    expect(slice_eq to be_true given(child->name, S("sub")));
    expect(child->type, == , DN_OBJECT);
    expect(child->object.size, == , 1);
    child = child->object.children;
    expect(child to not be_null);
    expect(slice_eq to be_true given(child->name, S("item")));
    expect(child->type, == , DN_STRING);
    expect(slice_eq to be_true given(child->value_str, S("value")));
  }

  it("can process an empty array") {
    dtree = dtree_from_json(S("[]"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_ARRAY);
    expect(dtree->root->array.size to be_zero);
  }

  it("can process an empty array containing an empty object") {
    dtree = dtree_from_json(S("[{}]"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_ARRAY);
    expect(dtree->root->array.size to be_one);
    expect(dtree->root->array.elem_type, == , DN_OBJECT);
    expect(dtree->root->array.nodes[0].type, == , DN_OBJECT);
    expect(dtree->root->array.nodes[0].object.size to be_zero);
  }

  it("can process an array of ints") {
    dtree = dtree_from_json(S("[1, 2, 3, 4, 5]"));
    expect(dtree to not be_null);
    expect(dtree->root to not be_null);
    expect(dtree->root->type, == , DN_ARRAY);
    expect(dtree->root->array.size, == , 5);
    expect(dtree->root->array.elem_type, == , DN_INT);
    expect(c_array(dtree->root->array.ints, 5, int64_t)
      to all_be( == , ((int64_t[]) { 1, 2, 3, 4, 5 }) [n] ));
  }

  after{
    expect(dtree to not be_null);
    dtree_delete(&dtree);
    expect(dtree to be_null);
  }

}

test_suite(tests_data_tree) {
  test_group(dtree_copy),
  test_group(dtree_from_json),
  test_suite_end
};

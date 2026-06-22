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

#define CSPEC_CUSTOM_TYPES \
  slice_t: "slice_t",       \
  DataTree: "dtree_t*", DataNode: "dnode_t*", default: "void*"

#include "cspec.h"

describe(dnode_read) {
  dnode_value_t result;

  context("when the tree has only a 'null' node") {

    // null
    DataNode subject = &(dnode_t) {
      .type = DN_NULL,
    };

    it("detects that there is a node at the root") {
      expect(dnode_contains to be_true given(subject, slice_empty));
      expect(dnode_contains to be_true given(subject, S("$")));
    }

    it("recognizes a slash after empty as the next level down") {
      expect(dnode_contains to be_false given(subject, S("/x")));
    }

    it("doesn't find something keyed below") {
      expect(dnode_contains to be_false given(subject, S("hi")));
    }

    it("doesn't find anything with a name with a slash") {
      expect(dnode_contains to be_false given(subject, S("a/b")));
    }

    it("gets the correct type from a null node") {
      expect(dnode_read to be_true given(subject, slice_empty, &result));
      expect(dnode_read to be_true given(subject, S("$"), &result));
      expect(result.type, == , DN_NULL);
    }
  }

  context("when the root node is a number/plain value") {

    DataNode subject = &(dnode_t) {
      .type = DN_INT,
      .value_int = 7,
    };

    it("only reads the top value with an empty identifier") {
      expect(dnode_contains to be_true given(subject, slice_empty));
      expect(dnode_read to be_false given(subject, S("/"), &result));
    }

    it("can read out the stored number") {
      expect(dnode_read to be_true given(subject, slice_empty, &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 7);
    }

  }

  context("when the root is an empty object") {

    // {}
    DataNode dnode = &(dnode_t) {
      .type = DN_OBJECT,
    };

    it("has zero children") {

    }

  }



  it("can represent an array") {

    // [ 1, 2, 3 ]
    DataNode dnode = &(dnode_t) {
      .type = DN_ARRAY,
      .array.elem_type = DN_INT,
      .array.size = 3,
      .array.ints = (int64_t[3]) { 1, 2, 3 }
    };
  }

  context("when the root is an object with some member values") {

    // { first: 1, second: 2, third: 3 }
    DataNode subject = &(dnode_t) {
      .type = DN_OBJECT,
      .object.size = 3,
      .object.children = (dnode_member_t[3]) {
        {
          .name = S("first"),
          .type = DN_INT,
          .value_int = 1,
        },
        {
          .name = S("second"),
          .type = DN_INT,
          .value_int = 2,
        },
        {
          .name = S("third"),
          .type = DN_FLOAT,
          .value_float = 3.5,
        },
      }
    };

    it("can access the properties of the root element") {
      expect(dnode_read to be_true given(subject, slice_empty, &result));
      expect(result.type, == , DN_OBJECT);
      expect(result.node->object.size, == , 3);
    }

    it("fails to access when there are children but the name is wrong") {
      expect(dnode_contains to be_false given(subject, S("forst")));
    }

    it("cannot access children of nodes with no children") {
      expect(dnode_contains to be_false given(subject, S("first/")));
    }

    it("can access child members of the root node") {
      expect(dnode_read to be_true given(subject, S("first"), &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 1);

      expect(dnode_read to be_true given(subject, S("third"), &result));
      expect(result.type, == , DN_FLOAT);
      expect(*result.value_float, == , 3.5);
    }

  }

  context("when accessing a multi-layer object mapping") {
    // { first: { second: { third: 3 } }, fourth: 4 }
    DataNode dnode = &(dnode_t) {
      .type = DN_OBJECT,
      .object.size = 2,
      .object.children = (dnode_member_t[2]) {
        {
          .name = S("first"),
          .type = DN_OBJECT,
          .object.size = 1,
          .object.children = (dnode_member_t[1]) {
            {
              .name = S("second"),
              .type = DN_OBJECT,
              .object.size = 1,
              .object.children = (dnode_member_t[]) {
                {
                  .name = S("third"),
                  .type = DN_INT,
                  .value_int = 3,
                }
              }
            }
          }
        },
        {
          .name = S("fourth"),
          .type = DN_INT,
          .value_int = 4
        },
      }
    };

    dnode->object.children[1].value_int++;

    it("does a thing") {
      int64_t* value = dnode_ref_int(dnode, S("$.first.second.third"));
      expect(value to not be_null);
      expect(*value, == , 3);
    }
  }

}

describe(dtree_new) {

  DataTree dtree = NULL;

  it("Should create a tree with a single node and delete successfully") {
    //dtree = dtree_new(DN_NULL);
  }

  after {
    dtree_delete(&dtree);
    expect(dtree to be_null);
  }

}

test_suite(tests_data_tree) {
  test_group(dnode_read),
  test_group(dtree_new),
  test_suite_end
};

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
      expect(dnode_contains to be_false given(subject, S("x")));
    }

    it("doesn't find something keyed below") {
      expect(dnode_contains to be_false given(subject, S("hi")));
    }

    it("doesn't find anything with a name with a slash") {
      expect(dnode_contains to be_false given(subject, S("$.b")));
    }

    it("won't index into a non-indexable node") {
      expect(dnode_contains to be_false given(subject, S("S[0]")));
      expect(dnode_contains to be_false given(subject, S("$['key']")));
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

    it("can read the top level node using the '$' specifier or empty") {
      expect(dnode_contains to be_true given(subject, S("$")));
      expect(dnode_contains to be_true given(subject, slice_empty));
      expect(dnode_read to be_false given(subject, S("$."), &result));
    }

    it("can read out the stored number") {
      expect(dnode_read to be_true given(subject, slice_empty, &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 7);
    }

  }

  context("when the root is an empty object") {

    // {}
    DataNode subject = &(dnode_t) {
      .type = DN_OBJECT,
      .object.size = 0, // can't omit because of padding nonsense
    };

    it("has zero children") {
      expect(dnode_read to be_true given(subject, S("$"), &result));
      expect(result.type, == , DN_OBJECT);
      expect(result.node->object.size to be_zero);
    }

  }

  context("when the root is an array of basic types") {

    // [ 1, 2, 3 ]
    DataNode subject = &(dnode_t) {
      .type = DN_ARRAY,
      .array.elem_type = DN_INT,
      .array.size = 3,
      .array.ints = (int64_t[3]) { 1, 2, 3 }
    };

    it("fails when bracket sections aren't closed") {
      expect(dnode_contains to be_false given(subject, S("$[1")));
    }

    it("fails when reading out of bounds") {
      expect(dnode_contains to be_false given(subject, S("$[3]")));
      expect(dnode_contains to be_false given(subject, S("$[-8]")));
    }

    it("can read a value from the array") {
      expect(dnode_read to be_true given(subject, S("$[0]"), &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 1);
    }

    it("can handle arbitrary spaces") {
      expect(dnode_contains to be_true given(subject, S("$[  1   ]")));
    }

    it("can access with negative indexing") {
      expect(dnode_read to be_true given(subject, S("$[-1]"), &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 3);

      expect(dnode_read to be_true given(subject, S("$[-3]"), &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 1);
    }

  }

  context("when the root is an array of strings") {

    DataNode subject = &(dnode_t) {
      .type = DN_ARRAY,
      .array.elem_type = DN_STRING,
      .array.size = 4,
      .array.strings = (slice_t[4]){
        S("first"), S("second"), S("third"), S("fourth")
      }
    };

    it("can find and access the string") {
      expect(dnode_read to be_true given(subject, S("$[0]"), &result));
      expect(result.type, == , DN_STRING);
      expect(slice_eq to be_true given(*result.value_str, S("first")));

      expect(dnode_read to be_true given(subject, S("$[-2]"), &result));
      expect(result.type, == , DN_STRING);
      expect(slice_eq to be_true given(*result.value_str, S("third")));
    }

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
      expect(dnode_contains to be_false given(subject, S("first.second")));
    }

    it("can access child members of the root node") {
      expect(dnode_read to be_true given(subject, S("first"), &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 1);

      expect(dnode_read to be_true given(subject, S("third"), &result));
      expect(result.type, == , DN_FLOAT);
      expect(*result.value_float, == , 3.5);
    }

    it("can access child nodes using index notation") {
      expect(dnode_read to be_true given(subject, S("$['first']"), &result));
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 1);

      expect(dnode_read to be_true given(subject, S("$['third']"), &result));
      expect(result.type, == , DN_FLOAT);
      expect(*result.value_float, == , 3.5);
    }

    it("can't use index notation with no closing quote or bracket") {
      expect(dnode_contains to be_false given(subject, S("$[first]")));
      expect(dnode_contains to be_false given(subject, S("$['first]")));
      expect(dnode_contains to be_false given(subject, S("$['first'")));
      expect(dnode_contains to be_false given(subject, S("$[first']")));
    }

    it("ignores spacing around index tokens") {
      expect(dnode_contains to be_true given(subject, S("$[ 'first']")));
      expect(dnode_contains to be_true given(subject, S("$['first' ]")));
      expect(dnode_contains to be_true given(subject, S("$ ['first']")));
      expect(dnode_contains to be_true given(subject, S("$['first'] ")));
      expect(dnode_contains to be_true given(subject, S("$ \t [  'first' ] ")));
    }

    it("does not ignore spacing inside the quotes") {
      expect(dnode_contains to be_false given(subject, S("$[' first']")));
      expect(dnode_contains to be_false given(subject, S("$['first ']")));
    }

  }

  context("when accessing a multi-layer object mapping") {

    // { first: { second: { third: 3 } }, fourth: 4 }
    DataNode subject = &(dnode_t) {
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
              .object.children = (dnode_member_t[1]) {
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

    it("can find the sub-objects") {
      expect(dnode_contains to be_true given(subject, S("first")));
      expect(dnode_contains to be_true given(subject, S("first.second")));
      expect(
        dnode_read to be_true given(subject, S("first.second.third"), &result)
      );
      expect(result.type, == , DN_INT);
      expect(*result.value_int, == , 3);
    }

    it("ignores leading spaces on dots") {
      expect(dnode_contains to be_true given(subject, S("first  .second")));
    }

    it("ignores trailing spaces as well") {
      // technically not in compliance with actual JSONPath, but that's fine
      expect(dnode_contains to be_true given(subject, S("first.  second")));
      expect(dnode_contains to be_true given(subject, S("first . second")));
    }

    it("can mix index and dot notation when indexing") {
      expect(
        dnode_contains to be_true given(subject, S("first['second'].third"))
      );
    }

  }

  context("when accessing an array of objects") {

    // {
    //    admin: { first_name: "Curtis", last_name: "McCoy", id: 123 },
    //    user : { first_name: "John"  , last_name: "Doe"  , id: 456 },
    // }
    DataNode subject = &(dnode_t) {
      .type = DN_ARRAY,
      .array.elem_type = DN_OBJECT,
      .array.size = 2,
      .array.nodes = (dnode_t[2]) {
        {
          .type = DN_OBJECT,
          .object.size = 3,
          .object.children = (dnode_member_t[3]) {
            {
              .name = S("first_name"),
              .type = DN_STRING,
              .value_str = S("Curtis"),
            },
            {
              .name = S("last_name"),
              .type = DN_STRING,
              .value_str = S("McCoy"),
            },
            {
              .name = S("id"),
              .type = DN_INT,
              .value_int = 123,
            },
          }
        },
        {
          .type = DN_OBJECT,
          .object.size = 3,
          .object.children = (dnode_member_t[3]) {
            {
              .name = S("first_name"),
              .type = DN_STRING,
              .value_str = S("John"),
            },
            {
              .name = S("last_name"),
              .type = DN_STRING,
              .value_str = S("Doe"),
            },
            {
              .name = S("id"),
              .type = DN_INT,
              .value_int = 456
            },
          }
        },
      }
    };

    it("can index through the array") {
      expect(dnode_contains to be_true given(subject, S("$[0].id")));
      expect(dnode_contains to be_true given(subject, S("$[1].id")));
    }

    it("correctly indexes and resolves types") {
      expect(
        dnode_read to be_true given(subject, S("$[0].last_name"), &result)
      );
      expect(result.type, == , DN_STRING);
      expect(slice_eq to be_true given(*result.value_str, S("McCoy")));
    }

    // TODO: conditional searches
    //    "$[?@.id == 456].first_name" => "John"

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

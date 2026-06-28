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

#include "data_node.h"

#define CSPEC_CUSTOM_TYPES \
  slice_t: "slice_t",       \
  DataNode: "dnode_t*", default: "void*"

#include "cspec.h"

describe(dnode_read) {
  dnode_value_t result;

  context("when the tree has only a 'null' node") {

    // null
    DataNode subject = NODE_ROOT(
      NODE_NULL
    );

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

    DataNode subject = NODE_ROOT(
      NODE_INT(7)
    );

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
    DataNode subject = NODE_ROOT(
      NODE_OBJECT_EMPTY
    );

    it("has zero children") {
      expect(dnode_read to be_true given(subject, S("$"), &result));
      expect(result.type, == , DN_OBJECT);
      expect(result.node->object.size to be_zero);
    }

  }

  context("when the root is an array of basic types") {

    // [ 1, 2, 3 ]
    DataNode subject = NODE_ROOT(
      NODE_ARRAY_INT(
        1, 2, 3
      )
    );

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
    DataNode subject = NODE_ROOT(
      NODE_OBJECT(
        MEMB_INT(S("first"), 1),
        MEMB_INT(S("second"), 2),
        MEMB_FLOAT(S("third"), 3.5)
      )
    );

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
    DataNode subject = NODE_ROOT(
      NODE_OBJECT(
        MEMB_OBJECT(S("first"),
          MEMB_OBJECT(S("second"),
            MEMB_INT(S("third"), 3)
          )
        ),
        MEMB_INT(S("fourth"), 4)
      )
    );

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

    DataNode subject = NODE_ROOT(
      NODE_ARRAY(
        NODE_OBJECT(
          MEMB_STRING(S("first_name"), S("Curtis")),
          MEMB_STRING(S("last_name"), S("McCoy")),
          MEMB_INT(S("id"), 123),
        ),
        NODE_OBJECT(
          MEMB_STRING(S("first_name"), S("John")),
          MEMB_STRING(S("last_name"), S("Doe")),
          MEMB_INT(S("id"), 456),
        )
      )
    );

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

describe(dnode_ref) {

  DataNode subject = NODE_ROOT(
    NODE_OBJECT(
      MEMB_BOOL(S("bool"), true),
      MEMB_INT(S("int"), 24),
      MEMB_FLOAT(S("float"), 36.2),
      MEMB_STRING(S("string"), S("It's actually a slice")),
      MEMB_OBJECT(S("object"), 
        MEMB_ARRAY(S("sub-array"),
          NODE_BOOL(true),
          NODE_INT(12),
          NODE_FLOAT(12.3),
          NODE_STRING(S("again")),
          NODE_OBJECT_EMPTY,
          NODE_ARRAY(
            NODE_NULL,
            NODE_NULL,
            NODE_INT(42069),
            NODE_NULL
          )
        )
      ),
      MEMB_ARRAY(S("mixed"),
        NODE_BOOL(true),
        NODE_INT(42),
        NODE_FLOAT(98.6),
        NODE_STRING(S("-1234")),
        NODE_NULL
      )
    )
  );

  context("can read matching types correctly") {

    it("works on booleans") {
      bool* btest = dnode_ref_bool(subject, S("bool"));
      expect(btest to not be_null);
      expect(*btest to be_true);
    }

    it("works on ints") {
      int64_t* itest = dnode_ref_int(subject, S("int"));
      expect(itest to not be_null);
      expect(*itest, == , 24);
    }

    it("works on floats") {
      double* ftest = dnode_ref_float(subject, S("float"));
      expect(ftest to not be_null);
      expect(*ftest, == , 36.2);
    }

    it("works on strings") {
      slice_t* stest = dnode_ref_str(subject, S("string"));
      expect(stest to not be_null);
      expect(slice_eq to be_true given(*stest, S("It's actually a slice")));
    }

    it("works for homogeneous array accessors") {
      expect(dnode_ref_bool to not be_null given(subject, S("mixed[0]")));
      expect(dnode_ref_int to not be_null given(subject, S("mixed[1]")));
      expect(dnode_ref_float to not be_null given(subject, S("mixed[2]")));
      expect(dnode_ref_str to not be_null given(subject, S("mixed[3]")));
    }

  }

  context("will return null on non-matching types") {

    it("works for booleans") {
      expect(dnode_ref_bool to not be_null given(subject, S("bool")));
      expect(dnode_ref_bool to be_null given(subject, S("int")));
      expect(dnode_ref_bool to be_null given(subject, S("float")));
      expect(dnode_ref_bool to be_null given(subject, S("string")));
      expect(dnode_ref_bool to be_null given(subject, S("object")));
      expect(dnode_ref_bool to be_null given(subject, S("object.sub-array")));
      expect(dnode_ref_bool to be_null given(subject, S("mixed[1]")));
      expect(dnode_ref_bool to be_null given(subject, S("mixed[21]")));
      expect(dnode_ref_bool to be_null given(subject, S("mixed[3]")));
      expect(dnode_ref_bool to be_null given(subject, S("mixed[4]")));
    }

    it("works for booleans") {
      expect(dnode_ref_int to be_null given(subject, S("bool")));
      expect(dnode_ref_int to not be_null given(subject, S("int")));
      expect(dnode_ref_int to be_null given(subject, S("float")));
      expect(dnode_ref_int to be_null given(subject, S("string")));
      expect(dnode_ref_int to be_null given(subject, S("object")));
      expect(dnode_ref_int to be_null given(subject, S("object.sub-array")));
      expect(dnode_ref_int to be_null given(subject, S("mixed[0]")));
      expect(dnode_ref_int to be_null given(subject, S("mixed[21]")));
      expect(dnode_ref_int to be_null given(subject, S("mixed[3]")));
      expect(dnode_ref_int to be_null given(subject, S("mixed[4]")));
    }

    it("works for floats") {
      expect(dnode_ref_float to be_null given(subject, S("bool")));
      expect(dnode_ref_float to be_null given(subject, S("int")));
      expect(dnode_ref_float to not be_null given(subject, S("float")));
      expect(dnode_ref_float to be_null given(subject, S("string")));
      expect(dnode_ref_float to be_null given(subject, S("object")));
      expect(dnode_ref_float to be_null given(subject, S("object.sub-array")));
      expect(dnode_ref_float to be_null given(subject, S("mixed[01]")));
      expect(dnode_ref_float to be_null given(subject, S("mixed[1]")));
      expect(dnode_ref_float to be_null given(subject, S("mixed[3]")));
      expect(dnode_ref_float to be_null given(subject, S("mixed[4]")));
    }

    it("works for strings") {
      expect(dnode_ref_str to be_null given(subject, S("bool")));
      expect(dnode_ref_str to be_null given(subject, S("int")));
      expect(dnode_ref_str to be_null given(subject, S("float")));
      expect(dnode_ref_str to not be_null given(subject, S("string")));
      expect(dnode_ref_str to be_null given(subject, S("object")));
      expect(dnode_ref_str to be_null given(subject, S("object.sub-array")));
      expect(dnode_ref_str to be_null given(subject, S("mixed[0]")));
      expect(dnode_ref_str to be_null given(subject, S("mixed[1]")));
      expect(dnode_ref_str to be_null given(subject, S("mixed[21]")));
      expect(dnode_ref_str to be_null given(subject, S("mixed[4]")));
    }

  }

}

describe(dnode_get) {

  DataNode subject = NODE_ROOT(
    NODE_OBJECT(
      MEMB_BOOL(S("bool"), true),
      MEMB_INT(S("int"), 24),
      MEMB_FLOAT(S("float"), 36.2),
      MEMB_STRING(S("string"), S("it's actually a slice")),
      MEMB_NULL(S("null")),
      MEMB_OBJECT(S("object"), 
        MEMB_ARRAY(S("sub-array"),
          NODE_BOOL(true),
          NODE_INT(12),
          NODE_FLOAT(12.3),
          NODE_STRING(S("True")),
          NODE_OBJECT_EMPTY,
          NODE_ARRAY(
            NODE_STRING(S("-987.5")),
            NODE_NULL,
            NODE_INT(42069),
            NODE_NULL
          )
        )
      )
    )
  );

  context("will return matching types correctly") {

    it("works for bools") {
      expect(dnode_get_bool to be_true given(subject, S("bool")));
      expect(dnode_get_bool to be_true given(subject, S("$.object.sub-array[0]")));
    }

    it("works for ints") {
      expect(dnode_get_int to be( == , 24) given(subject, S("int")));
      expect(dnode_get_int to be( == , 12)
        given(subject, S("object.sub-array[1]")));
      expect(dnode_get_int to be( == , 42069)
        given(subject, S("object.sub-array[5][2]")));
    }

    it("works for floats") {
      expect(dnode_get_float to be_about(36.2) given(subject, S("float")));
      expect(dnode_get_float to be_about(12.3)
        given(subject, S("object.sub-array[2]")));
    }

    it("works for strings") {
      slice_t result = dnode_get_str(subject, S("string"));
      expect(slice_eq to be_true given(result, S("it's actually a slice")));
      result = dnode_get_str(subject, S("object.sub-array[3]"));
      expect(slice_eq to be_true given(result, S("True")));
      result = dnode_get_str(subject, S("$.object.sub-array[5][0]"));
      expect(slice_eq to be_true given(result, S("-987.5")));
    }

  }

  context("will up-convert some types when they don't match") {

    it("works for bools") {
      expect(dnode_get_bool to be_true given(subject, S("int")));
      expect(dnode_get_bool to be_true given(subject, S("float")));
      expect(dnode_get_bool to be_true given(subject, S("object.sub-array[3]")));
    }

    it("works for ints") {
      expect(dnode_get_int to be( == , 36) given(subject, S("float")));
      expect(dnode_get_int to be( == , 1) given(subject, S("bool")));
      expect(dnode_get_int to be( == , 0) given(subject, S("string")));

      // converting from bool
      expect(dnode_get_int to be( == , 12)
        given(subject, S("$.object.sub-array[1]")));

      // converting from float
      expect(dnode_get_int to be( == , 12)
        given(subject, S("$.object.sub-array[2]")));

      // converting from string containing value
      expect(dnode_get_int to be( == , -987)
        given(subject, S("object.sub-array[5][0]")));
    }

    it("works for floats") {
      expect(dnode_get_float to be_about(1) given(subject, S("bool")));
      expect(dnode_get_float to be_about(24) given(subject, S("int")));
      expect(dnode_get_float to be_about(-987.5)
        given(subject, S("object.sub-array[5][0]")));
      expect(dnode_get_float to be( == , 42069)
        given(subject, S("object.sub-array[5][2]")));
    }

    it("only really works for strings when converting bools") {
      slice_t result = dnode_get_str(subject, S("bool"));
      expect(slice_eq to be_true given(result, slice_true));
    }

  }

  context("defaults to a zero-equivalent when the value isn't present") {

    it("works for bools") {
      expect(dnode_get_bool to be_false given(subject, S("invalid")));
      expect(dnode_get_bool to be_false given(subject, S("null")));
    }

    it("works for ints") {
      expect(dnode_get_int to be( == , 0) given(subject, S("invalid")));
      expect(dnode_get_int to be( == , 0) given(subject, S("null")));
    }

    it("works for floats") {
      expect(dnode_get_int to be( == , 0) given(subject, S("invalid")));
      expect(dnode_get_int to be( == , 0) given(subject, S("null")));
    }

    it("works for strings") {
      slice_t result = dnode_get_str(subject, S("int"));
      expect(slice_eq to be_true given(result, slice_empty));
      result = dnode_get_str(subject, S("float"));
      expect(slice_eq to be_true given(result, slice_empty));
      result = dnode_get_str(subject, S("null"));
      expect(slice_eq to be_true given(result, slice_empty));
    }

  }

}

describe(dnode_select) {

  DataNode subject = NODE_ROOT(
    NODE_OBJECT(
      MEMB_BOOL(S("bool"), true),
      MEMB_INT(S("int"), 24),
      MEMB_FLOAT(S("float"), 36.2),
      MEMB_STRING(S("string"), S("it's actually a slice")),
      MEMB_NULL(S("null")),
      MEMB_OBJECT(S("object"),
        MEMB_ARRAY(S("sub-array"),
          NODE_BOOL(true),
          NODE_INT(12),
          NODE_FLOAT(12.3),
          NODE_STRING(S("True")),
          NODE_OBJECT_EMPTY,
          NODE_ARRAY(
            NODE_STRING(S("-987.5")),
            NODE_NULL,
            NODE_INT(42069),
            NODE_NULL
          )
        )
      )
    )
  );

  context("using a simple mask of two leaf nodes") {

    DataNode result = dnode_select(subject, NODE_ROOT(
      NODE_OBJECT(
        MEMB_INT(S("int"), 10),
        MEMB_FLOAT(S("float"), 0.0)
      )
    ));

    it("can copy a value into a query 'mask' structure") {
      expect(dnode_get_int to be( == , 24) given(result, S("int")));
      expect(dnode_get_float to be_about(36.2) given(result, S("float")));
    }

    it("will not find values that weren't in the filter") {
      expect(dnode_ref_bool to be_null given(result, S("bool")));
      expect(dnode_contains to be_false given(result, S("object")));
    }

  }

  context("using a mask with extra keys that aren't found in the source") {

    DataNode result = dnode_select(subject, NODE_ROOT(
      NODE_OBJECT(
        MEMB_INT(S("unknown"), 12),
        MEMB_FLOAT(S("asdf"), 12.34)
      )
    ));

    it("uses the provided default value") {
      expect(dnode_contains to be_false given(subject, S("unknown")));
      expect(dnode_get_int to be( == , 12) given(result, S("unknown")));

      expect(dnode_contains to be_false given(subject, S("asdf")));
      expect(dnode_get_float to be_about(12.34) given(result, S("asdf")));
    }

  }

  context("using a mask that converts values") {

    DataNode result = dnode_select(subject, NODE_ROOT(
      NODE_OBJECT(
        MEMB_BOOL(S("int"), false),
        MEMB_INT(S("float"), 0),
        MEMB_FLOAT(S("bool"), 12.0)
      )
    ));

    it("converts the values correctly") {
      expect(dnode_get_type to be( == , DN_BOOL) given(result, S("$['int']")));
      expect(dnode_get_bool to be_true given(result, S("int")));

      expect(dnode_get_type to be(== , DN_INT) given(result, S("float")));
      expect(dnode_get_int to be( == , 36) given(result, S("$.float")));

      expect(dnode_get_type to be(== , DN_FLOAT) given(result, S("bool")));
      expect(dnode_get_float to be_about(1.0) given(result, S("int")));
    }

  }

  context("can mask values in an array") {

    DataNode result = dnode_select(subject, NODE_ROOT(
      NODE_OBJECT(
        MEMB_OBJECT(S("object"),
          MEMB_ARRAY(S("sub-array"),
            NODE_INT(0),
            NODE_FLOAT(0),
            NODE_NULL,
            NODE_BOOL(false),
            NODE_NULL,
            NODE_ARRAY_FLOAT(10, 10, 10)
          )
        )
      )
    ));

    it("correctly filters out the members of the array") {
      slice_t first = S("object.sub-array[0]");
      slice_t second = S("object.sub-array[1]");
      slice_t third = S("object.sub-array[2]");
      slice_t fourth = S("object.sub-array[3]");
      slice_t fifth = S("object.sub-array[4]");
      slice_t sixth = S("object.sub-array[5]");

      expect(dnode_get_type to match(DN_INT) given(result, first));
      expect(dnode_get_type to match(DN_FLOAT) given(result, second));
      expect(dnode_get_type to match(DN_FLOAT) given(result, third));
      expect(dnode_get_type to match(DN_BOOL) given(result, fourth));
      expect(dnode_get_type to match(DN_NULL) given(result, fifth));
      expect(dnode_get_type to match(DN_ARRAY) given(result, sixth));

      expect(dnode_get_int to be( == , 1) given(result, first));
      expect(dnode_get_float to be_about(12) given(result, second));
      expect(dnode_get_float to be_about(12.3) given(result, third));
      expect(dnode_get_bool to be_true given(result, fourth));
    }

    it("filters the values in the sub-array into floats") {
      slice_t first = S("object.sub-array[5][0]");
      slice_t second = S("object.sub-array[5][1]");
      slice_t third = S("object.sub-array[5][2]");

      expect(dnode_get_type to match(DN_FLOAT) given(result, first));
      expect(dnode_get_type to match(DN_FLOAT) given(result, second));
      expect(dnode_get_type to match(DN_FLOAT) given(result, third));

      expect(dnode_get_float to be_about(-987.5) given(result, first));
      expect(dnode_get_float to be_about(10) given(result, second));
      expect(dnode_get_float to be_about(42069) given(result, third));
    }

  }

}

test_suite(tests_data_node) {
  test_group(dnode_read),
  test_group(dnode_ref),
  test_group(dnode_get),
  test_group(dnode_select),
  test_suite_end
};

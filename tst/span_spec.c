/*******************************************************************************
* MIT License
*
* Copyright (c) 2024 Curtis McCoy
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

static int cmp_int_vptr(const void* a, const void* b) {
  return *(int*)a - *(int*)b;
}

#define con_type int
#define con_cmp cmp_int_vptr
#include "span.h"
#undef con_type
#undef con_cmp

#include "cspec.h"

describe(span_sizes) {

  int data[] = { 32, 58, 3874, 201, 3, 9841 };
  index_t count = ARRAY_COUNT(data);
  index_t size_bytes = sizeof(data);

  context("using a generic typeless span") {

    span_t span = { .begin = data, .end = data + count };

    it("creats a span view of a plain c_array") {
      expect(span_size(span, sizeof(*data)), == , count);
      expect(span_size_bytes(span), == , size_bytes);
    }

  }

  context("using a typed span") {

    span_int_t span = span_int(data, data + count);

    it("gets the size of the span with no explicit size input") {
      expect(span_int_size(span), == , count);
      expect(span_int_size_bytes(span), == , size_bytes);
    }

  }

}

describe(span_set_bytes) {

  int data[] = { 32, 58, 3874, 201, 3, 9841 };

  context("using a generic span") {

    span_t span = { .begin = data, .end = data + ARRAY_COUNT(data) };

    it("replaces an array's data with zeroes") {
      expect(data to all_be( != , 0 , c_array));
      span_set_bytes(span, 0);
      expect(data to all_be( == , 0 , c_array));
    }

    /*
    it("can use a different type for the generic span") {
      expect(span to not all_be( == , 5 , span));
      expect(data to not all_be( == , 5 , c_array));
      span_set_bytes(span, 5);
      expect(span to all_be( == , 5 , span));
      expect(data to all_be( == , 5 , c_array));
    }
    */

  }

  context("using a typed span") {

    span_int_t span = span_int(data, data + ARRAY_COUNT(data));

    it("replaces an array's data with a single character") {
      expect(data to all_be( != , 0));
      span_int_set_bytes(span, 0);
      expect(data to all_be( == , 0));
    }

    it("operates on bytes rather than at the integer level") {
      expect(data to all_be( != , (int)0xAAAAAAAA));
      span_int_set_bytes(span, 0xAA);
      expect(data to all_be(== , (int)0xAAAAAAAA));
    }

  }

}

describe(span_eq) {

  int data1[] = { 6, 3, 4, 2, 1, 5 };
  int data2[] = { 1, 2, 3, 4, 5, 6 };
  int data3[] = { 1, 2, 3, 4, 5, 6 };

  // arrays of pointers to test shallow vs deep comapre
  //int* ptr1[] = { &data1[4], &data2[0] };
  //int* ptr2[] = { &data2[0], &data3[0] };

  context("using a generic span") {

    span_t span1 = SPAN(data1);
    span_t span2 = SPAN(data2);
    span_t span3 = SPAN(data3);

    it("tests for basic equality between spans") {
      expect(span_eq(span2, span3) to be_true);
      expect(span_eq(span1, span2) to be_false);
    }

  }

  context("using a typed span") {

    span_int_t span1 = SPAN(data1);
    span_int_t span2 = SPAN(data2);
    span_int_t span3 = SPAN(data3);

    it("tests for basic equality") {
      expect(span_int_eq(span2, span3) to be_true);
      expect(span_int_eq(span1, span2) to be_false);
    }

  }

}

describe(span_sort) {

  int data[] = { 32, 58, 3874, 201, 3, 9841, 111 };
  int sorted[] = { 3, 32, 58, 111, 201, 3874, 9841 };

  context("using a generic span") {

    span_t span = { .begin = data, .end = data + ARRAY_COUNT(data) };

    it ("sorts the data from lowest to highest using the compare fn") {
      span_sort(span, sizeof(int), cmp_int_vptr);
      expect(data to all_be( == , sorted[n]), int);
      expect(data to match(sorted));
    }

  }

  context("using a typed span") {

    it ("sorts the data from lowest to highest with implicit compare fn") {
      span_int_t span = { .begin = data, .end = data + ARRAY_COUNT(data) };
      span_int_sort(span);
      expect(data to all_be( == , sorted[n]));
    }

    it("can sort a partial segment of the span") {
      int sorted_part[] = { 32, 3, 58, 201, 3874, 9841, 111 };
      span_int_t span = SPAN(data);
      ++span.begin; --span.end;
      span_int_sort(span);
      expect(data to all_be( == , sorted_part[n]));
    }

  }

}

test_suite(tests_span) {
  test_group(span_sizes),
  test_group(span_set_bytes),
  test_group(span_eq),
  test_group(span_sort),
  test_suite_end
};

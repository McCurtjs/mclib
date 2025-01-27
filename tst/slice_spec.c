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

#include "slice.h"

#define CSPEC_CUSTOM_TYPES                                                    \
  slice_t: "slice_t", slice_t*: "slice_t*",                                   //

#include "cspec.h"

describe(slice_basic) {

  it("creates a slice from a literal") {
    slice_t slice = S("literal string slice");

    expect(slice.length, == , 20, index_t);
    expect(slice.size, == , 20, index_t);
  }

#ifdef __GNUC__
#pragma GCC diagnostic push
  // warns that the length and size are a union, which is the point
#pragma GCC diagnostic ignored "-Waddress"
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-bool-conversion"
#endif
  it("validates that length and size are union aliases") {
    slice_t slice = S("literal string slice");

    expect(&slice.length, == , &slice.size, csBool);
  }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif

  it("creates a range in a local fixed-size char array") {
    char c_str[] = "This is a stack string";
    slice_t slice = S(c_str);

    expect(slice.begin == c_str);
    expect(slice.length, == , 22);
  }

  it("creates a range within the given c-string string") {
    char* c_str = "This is a c-string";
    slice_t slice = slice_build(c_str);

    expect(slice.begin == c_str);
    expect(slice.length, == , 18);
  }

  it("creates a range using the _s version that specifies length") {
    char* c_str = "This is a c-string";
    slice_t slice = slice_build_s(c_str, 4);
    slice_t This = S("This");

    expect(slice.begin == c_str);
    expect(slice.length, == , 4);
    expect(slice to match(This, slice_eq));
  }

  expect(malloc_count, == , 0);
}

describe(slice_eq) {

  char str_1[] = "String";

  it("tests that two strings are bitwise equal") {
    char str_2[] = "String";

    // strings made on the stack shouldn't have the same address
    expect(&str_1[0] != &str_2[0]);

    slice_t slice1 = slice(str_1);
    slice_t slice2 = slice(str_2);

    expect(slice1 to match(slice2, slice_eq));
  }

  it("handles substring cases") {
    char str_2[] = "Strix";

    // Comparing "Stri" with "Stri"
    slice_t slice1 = slice_build_s(str_1, 4);
    slice_t slice2 = slice_build_s(str_2, 4);

    expect(slice1 to match(slice2, slice_eq));

    // Comparing "Strix" with "Strin"
    slice1 = slice_build_s(str_1, 5);
    slice2 = slice_build_s(str_2, 5);

    expect(slice1 to not match(slice2, slice_eq));

    // Comparing "Strix" with "Stri"
    slice2 = slice_build_s(str_1, 4);

    expect(slice1 to not match(slice2, slice_eq));
  }

  it("tests that two strings are not equal") {
    char str_2[] = "StrinG";

    // strings made on the stack shouldn't have the same address
    expect(&str_1[0] != &str_2[0]);

    slice_t slice1 = slice(str_1);
    slice_t slice2 = slice(str_2);

    expect(slice1 to not match(slice2, slice_eq));

  }

}

describe(slice_starts_with) {
  slice_t slice = S("This is a string");

  it("handles a basic true use case") {
    expect(slice_starts_with(slice, S("This")));
  }

  it("handles a basic false use case") {
    expect(not slice_starts_with(slice, S("Thos")));
  }

  it("is case sensitive") {
    expect(not slice_starts_with(slice, S("THIS")));
  }

  it("returns true given an empty string") {
    expect(slice_starts_with(slice, S("")));
  }

  it("returns true given the full string") {
    expect(slice_starts_with(slice, S("This is a string")));
  }

  it("returns false given more than the full string") {
    expect(not slice_starts_with(slice, S("This is a string with more")));
  }

}

describe(slice_ends_with) {
  slice_t slice = S("This is a string");

  it("handles a basic true use case") {
    expect(slice_ends_with(slice, S("string")));
  }

  it("handles a basic false use case") {
    expect(not slice_ends_with(slice, S("strong")));
  }

  it("is case sensitive") {
    expect(not slice_ends_with(slice, S("STRING")));
  }

  it("returns true given an empty string") {
    expect(slice_ends_with(slice, S("")));
  }

  it("returns true given the full string") {
    expect(slice_ends_with(slice, S("This is a string")));
  }

  it("returns false given more than the full string") {
    expect(not slice_ends_with(slice, S("And This is a string")));
  }

}

describe(slice_contains) {
  slice_t slice = S("This is a string");

  it("handles a basic true use case") {
    expect(slice to match(S("is a"), slice_contains));
  }

  it("handles a basic false use case") {
    expect(slice to not match(S("not in"), slice_contains));
  }

  it("is case sensitive") {
    expect(slice to not match(S("IS A"), slice_contains));
  }

  it("returns true given an empty string") {
    expect(slice to match(S(""), slice_contains));
  }

  it("returns true given the full string") {
    expect(slice to match(S("This is a string"), slice_contains));
  }

  it("returns false given more than the full string") {
    expect(slice to not match(S("This is a string."), slice_contains));
  }

}

describe(slice_contains_char) {

}

describe(slice_is_empty) {

  it("detects an empty string") {
    expect(slice_is_empty to be_true given(slice_empty));
  }

  it("detects a string with spaces") {
    expect(slice_is_empty to be_true given(S("   ")));
  }

  it("detects a string with tabs and newlines") {
    expect(slice_is_empty to be_true given(S("\t")));
    expect(slice_is_empty to be_true given(S("\n")));
    expect(slice_is_empty to be_true given(S("\t \n")));
  }

  it("detects a string that is not empty") {
    expect(slice_is_empty to be_false given(S("     \t.\n  ")));
  }

}

describe(slice_to_bool) {
  bool out = false, * p_out = &out;

  //it("fails when no output parameter is given") {
  //  p_out = NULL;
  //  expect(slice_to_bool to be_false given(slice_true, p_out));
  //}

  it("converts from exact strings") {
    expect(slice_to_bool to be_true given(S("true"), p_out));
    expect(out to be_true);

    expect(slice_to_bool to be_true given(S("false"), p_out));
    expect(out to be_false);

    expect(slice_to_bool to be_true given(slice_true, p_out));
    expect(out to be_true);

    expect(slice_to_bool to be_true given(slice_false, p_out));
    expect(out to be_false);
  }

  it("is case insensitive") {
    expect(slice_to_bool to be_true given(S("TRUE"), p_out));
    expect(out to be_true);

    expect(slice_to_bool to be_true given(S("FALSE"), p_out));
    expect(out to be_false);

    expect(slice_to_bool to be_true given(S("True"), p_out));
    expect(out to be_true);

    expect(slice_to_bool to be_true given(S("False"), p_out));
    expect(out to be_false);
  }

  it("fails to convert empty or too-short strings") {
    expect(slice_to_bool to be_false given(slice_empty, p_out));
    expect(slice_to_bool to be_false given(S("tru"), p_out));
    expect(slice_to_bool to be_false given(S("fals"), p_out));
  }

  it("does not accept leading spaces") {
    expect(slice_to_bool to be_false given(S(" true"), p_out));
    expect(slice_to_bool to be_false given(S(" false"), p_out));
  }

  it("fails to convert strings with non-matching characters") {
    expect(slice_to_bool to be_false given(S("trub"), p_out));
    expect(slice_to_bool to be_false given(S("falze"), p_out));
  }

  it("allows additional characters after the matching portion") {
    expect(slice_to_bool to be_true given(S("true stuff"), p_out));
    expect(out to be_true);

    expect(slice_to_bool to be_true given(S("false statement"), p_out));
    expect(out to be_false);
  }

}

describe(slice_to_int) {
  int out = 0, *p_out = &out;

  //it("fails when no output parameter is given") {
  //  p_out = NULL;
  //  expect(slice_to_int to be_false given(S("123235"), p_out));
  //}

  it("fails when input is empty") {
    expect(slice_to_int to be_false given(slice_empty, p_out));
  }

  it("converts unsinged numbers") {
    expect(slice_to_int to be_true given(S("0"), p_out));
    expect(out, == , 0);

    expect(slice_to_int to be_true given(S("10"), p_out));
    expect(out, == , 10);

    expect(slice_to_int to be_true given(S("123401234"), p_out));
    expect(out, == , 123401234);
  }

  it("converts signed numbers") {
    expect(slice_to_int to be_true given(S("+0"), p_out));
    expect(out, == , 0);

    expect(slice_to_int to be_true given(S("+10"), p_out));
    expect(out, == , 10);

    expect(slice_to_int to be_true given(S("+123401234"), p_out));
    expect(out, == , 123401234);

    expect(slice_to_int to be_true given(S("-1"), p_out));
    expect(out, == , -1);

    expect(slice_to_int to be_true given(S("-10"), p_out));
    expect(out, == , -10);

    expect(slice_to_int to be_true given(S("-7482934"), p_out));
    expect(out, == , -7482934);
  }

  it("allows trailing characters") {
    expect(slice_to_int to be_true given(S("0 items"), p_out));
    expect(out, == , 0);

    expect(slice_to_int to be_true given(S("10 monkeys"), p_out));
    expect(out, == , 10);

    expect(slice_to_int to be_true given(S("3985206 is a number"), p_out));
    expect(out, == , 3985206);
  }

  it("doesn't allow leading spaces") {
    expect(slice_to_int to be_false given(S(" 5"), p_out));
  }

  it("fails with non-numeric input") {
    expect(slice_to_int to be_false given(S("a5"), p_out));
  }

}

describe(slice_to_long) {

}

describe(slice_to_float) {

}

describe(slice_to_double) {

}

describe(slice_find) {
  /*
  slice_t range = S("This is a string");

  it("finds an index") {
    slice_t is = S("is");
    expect(slice_find(range, is), == , 2u);
  }

  it("fails to find a substring that isn't present") {
    slice_t np = S("not present");
    expect(slice_find(range, np), == , range.size);
  }

  it("can find the first word in the string") {
    slice_t This = S("This");
    expect(slice_find(range, This), == , 0u);
  }

  expect(malloc_count == 0);
  */
}

describe(slice_index_of) {
  slice_t str = S("This is a string");
  slice_t is = S("is");

  it("finds an index") {
    expect(slice_index_of(str, is, 0), == , 2u);
  }

  it("fails to find a substring that isn't present") {
    expect(slice_index_of(str, S("Not present"), 0), == , str.size);
  }

  it("begins the search from a given position") {
    expect(slice_index_of(str, is, 3), == , 5u);
  }

  it("fails to find a word in the string that starts before the start_pos") {
    expect(slice_index_of(str, is, 6), == , str.size);
  }

  it("fails to find anything if the start pos is after the end of the string") {
    expect(slice_index_of(str, is, 99), == , str.size);
  }

  it("looping end-pos can be used to track through the string") {
    index_s tracker = 0;

    tracker = slice_index_of(str, S("i"), tracker);
    slice_t result = islice_substring(str, tracker, str.size);
    expect(result to match(S("is is a string"), slice_eq));

    tracker = slice_index_of(str, S("i"), tracker + 1);
    result = slice_substring(str, tracker, str.size);
    expect(result to match(S("is a string"), slice_eq));

    tracker = slice_index_of(str, S("i"), tracker + 1);
    result = slice_substring(str, tracker, str.size);
    expect(result to match(S("ing"), slice_eq));

    tracker = slice_index_of(str, S("i"), tracker + 1);
    expect(tracker, == , str.size);
  }

}

describe(slice_token) {

}

describe(slice_substring) {

  slice_t base = S("This is a string");
  slice_t subject = slice_empty;
  slice_t expected = slice_empty;

  it("gets a substring from 0 to 0") {
    subject = slice_substring(base, 0, 0);
    expected = slice_empty;
  }

  it("gets substring of whole string") {
    subject = slice_substring(base, 0, base.size);
    expected = base;
  }

  it("gets a partial substring from the beginning") {
    subject = slice_substring(base, 0, 4);
    expected = S("This");
  }

  it("gets a substring starting partway in the string") {
    subject = slice_substring(base, 5, 9);
    expected = S("is a");
  }

  it("uses a negative offset for the start of the substring") {
    subject = slice_substring(base, -6);
    expected = S("string");
  }

  it("uses negative offsets for the start and end") {
    subject = slice_substring(base, -8, -3);
    expected = S("a str");
  }

  it("has a string start past the end") {
    subject = slice_substring(base, 20, 24);
    expected = slice_empty;
  }

  it("has a string end before the beginning") {
    subject = slice_substring(base, 5, 3);
    expected = slice_empty;
  }

  it("has a string end before the beginning") {
    subject = slice_substring(base, 1, 0);
    expected = slice_empty;
  }

  it("clamps to valid end of string") {
    subject = slice_substring(base, 0, 500);
    expected = base;
  }

  it("clamps the beginning of the string for negative values") {
    subject = slice_substring(base, -30);
    expected = base;
  }

  it("does not allow going past the end of the string") {
    subject = slice_substring(base, 10, 20);
    expected = S("string");
  }

  after {
    expect(subject to match(expected, slice_eq));
    expect(malloc_count == 0);
  }

}

describe(slice_trim) {

  slice_t slice = S("\t  String with extra spaces   \n");
  slice_t subject = slice_empty;
  slice_t expected = slice_empty;

  it("trims all leading and trailing spaces") {
    subject = slice_trim(slice);
    expected = S("String with extra spaces");
  }

  it("trims all leading spaces") {
    subject = slice_trim_start(slice);
    expected = S("String with extra spaces   \n");
  }

  it("trims all trailing spaces") {
    subject = slice_trim_end(slice);
    expected = S("\t  String with extra spaces");
  }

  it("Does nothing to a string with no leading or trailing spaces") {
    subject = slice_trim(slice_true);
    expected = S("true");
  }

  after {
    expect(subject to match(expected, slice_eq));
    expect(malloc_count == 0);
  }

}

describe(slice_conversion) {
  byte message[] = "This will be a slice!";
  span_byte_t span = SPAN(message);
  slice_t slice = span_byte_to_slice(span);

  it("allows the span data to be used as a slice") {
    slice = slice_substring(slice, 15, -1);
    expect(slice to match(S("slice"), slice_eq));
  }

  it("reflects the span data changing") {
    span.begin[2] = 'a';
    span.begin[3] = 't';
    slice_t expected = S("That will be a slice!");

    expect(slice to match(expected, slice_eq));
  }

}

describe(slice_split) {

  slice_t slice = S("This is, a collection, of strings");
  Array_slice result = NULL;

  it("performs a basic split on commas") {
    result = slice_split(slice, S(","));
    expect(result->size, == , 3u);

    slice_t expected[3] = { S("This is"), S(" a collection"), S(" of strings") };
    expect(result to all(slice_eq, expected[n], slice_t, array));
  }

  it("performs a multi-char split") {
    result = slice_split(slice, S(", "));
    expect(result->size, == , 3u);

    slice_t expected[3] = { S("This is"), S("a collection"), S("of strings") };
    expect(result to all(slice_eq, expected[n], slice_t, array));
  }

  it("splits on an empty string") {
    result = slice_split(slice, slice_empty);
    expect(result->size, == , slice.size);

    expect(result to all(1 == slice_size, slice_t, array));
    expect(result to all(slice_eq, slice_substring(slice, n, n + 1), slice_t, array));
  }

  it("tries to split on a delimiter that isn't present") {
    result = slice_split(slice, S("NOT_INCLUDED"));
    expect(result->size, == , 1u);
    expect(result to all(slice_eq, slice, slice_t, array));
  }

  it("tries to split with a delimiter that is too big") {
    result = slice_split(slice, S("This is, a collection, of strings, but more"));
    expect(result->size, == , 1u);
    expect(result to all(slice_eq, slice, slice_t, array));
  }

  it("splits using the same string as the delimiter") {
    result = slice_split(slice, slice);
    expect(result->size, == , 2u);
    expect(result to all(slice_eq, slice_empty, slice_t, array));
  }

  if (result) {
    arr_slice_delete(&result);
  }

}

test_suite(tests_slice) {
  test_group(slice_basic),
  test_group(slice_eq),
  test_group(slice_starts_with),
  test_group(slice_ends_with),
  test_group(slice_contains),
  test_group(slice_contains_char),
  test_group(slice_is_empty),
  test_group(slice_to_bool),
  test_group(slice_to_int),
  test_group(slice_to_long),
  test_group(slice_to_float),
  test_group(slice_to_double),
  test_group(slice_find),
  test_group(slice_index_of),
//test_group(slice_index_of_char),
  test_group(slice_token),
  test_group(slice_substring),
  test_group(slice_trim),
  test_group(slice_conversion),
  test_group(slice_split),
  test_suite_end
};

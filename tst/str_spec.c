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

#include "str.h"

#define CSPEC_CUSTOM_TYPES                                                    \
  StringRange: "StringRange", StringRange*: "StringRange*", String: "String", //

#include "cspec.h"

describe(str_range) {

  it("creates a range from a literal") {
    StringRange range = R("literal string range");

    expect(range.length, == , 20u);
    expect(range.size, == , 20u);
  }

#ifdef __GNUC__
#pragma GCC diagnostic push
  // warns that the length and size are a union, which is the point
#pragma GCC diagnostic ignored "-Waddress"
#endif
  it("validates that length and size are union aliases") {
    StringRange range = R("literal string range");

    expect(&range.length, == , &range.size, csBool);
  }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

  it("creates a range in a local fixed-size char array") {
    char c_str[] = "This is a stack string";
    StringRange range = R(c_str);

    expect(range.begin == c_str);
    expect(range.length, == , 22u);
  }

  it("creates a range within the given c-string string") {
    char* c_str = "This is a c-string";
    StringRange range = str_range(c_str);

    expect(range.begin == c_str);
    expect(range.length, == , 18u);
  }

  it("creates a range using the _s version that specifies length") {
    char* c_str = "This is a c-string";
    StringRange range = str_range_s(c_str, 4);

    expect(range.begin == c_str);
    expect(range.length, == , 4u);
    expect(range to match("This", str_eq));
  }

  expect(malloc_count, == , 0);

}

describe(str_new) {
  String subject = NULL;

  char* c_str = "This is a c-string";

  it("makes a copy of the c_string using malloc") {
    subject = str_new(c_str);
    expect(subject to match(c_str, str_eq));
  }

  it("makes a copy using the _s version") {
    subject = str_new_s(c_str, 4);
    expect(subject to match("This", str_eq));
  }

  it("ensures the direct and range pointers are the same/shared in union") {
    subject = str_new(c_str);
    expect(&subject->begin == &subject->range.begin);
    expect(&subject->size == &subject->range.size);
    expect(&subject->size == &subject->length);
  }

  if (subject) {
    expect(subject->begin != c_str);
    expect(malloc_count == 1);
    str_delete(&subject);
  }

}

describe(str_copy) {
  String subject = NULL;

  char c_str[] = "This is a string";
  char* p_str = c_str;
  StringRange range = R(c_str);

  it("allocates a new copy of the string directly from the c string") {
    subject = str_copy(c_str);
    expect(subject to match(p_str, str_eq));
    expect(subject->begin != c_str);
  }

  it("allocates a new copy of the string from a range") {
    subject = str_copy(range);
    expect(subject to match(range, str_eq));
    expect(subject->begin != range.begin);
  }

  it("allocates a new copy of the string from another dynamic string") {
    String str = str_new(c_str);
    subject = str_copy(str);
    expect(subject to match(str, str_eq));
    str_delete(&str);
  }

  if (subject) {
    expect(subject->begin != range.begin);
    str_delete(&subject);
  }

}

describe(str_from_bool) {
  String subject = NULL;

  it("gets a true value from a bool") {
    subject = str_from_bool(TRUE);
    expect(subject == str_true);
    expect(subject to match("true", str_eq));
  }

  it("gets a false value from a bool") {
    subject = str_from_bool(FALSE);
    expect(subject == str_false);
    expect(subject to match("false", str_eq));
  }

  if (subject) {
    expect(malloc_count == 0);
  }

}

describe(str_from_int) {
  String subject = NULL;

  it("gets an int from 0") {
    subject = str_from_int(0);
    expect(subject to match("0", str_eq));
  }

  it("converts from a positive integer") {
    subject = str_from_int(7);
    expect(subject to match("7", str_eq));
  }

  it("converts from a negative integer") {
    subject = str_from_int(-4);
    expect(subject to match("-4", str_eq));
  }

  it("tries a bigger number") {
    subject = str_from_int(1746);
    expect(subject to match("1746", str_eq));
  }

  it("tries a bigger negative number") {
    subject = str_from_int(-84756);
    expect(subject to match("-84756", str_eq));
  }

  if (subject) {
    str_delete(&subject);
  }

}

describe(str_from_float) {
  String subject = NULL;

  it("gets a float from 0") {
    subject = str_from_float(0.0f);
    expect(str_eq(subject->range, R("0")));
  }

  it("gets pretty exact values from whole numbers") {
    subject = str_from_float(4.0f);
    expect(str_eq(subject->range, R("4")));
  }

  it("gets pretty exact values from negative whole numbers") {
    subject = str_from_float(-9.0f);
    expect(str_eq(subject->range, R("-9")));
  }

  it("gets decimal values") {
    subject = str_from_float(2.73f);
    expect(str_eq(subject->range, R("2.73")));
  }

  if (subject) {
    str_delete(&subject);
  }

}

describe(str_delete) {

  it("frees the memory and zeroes the pointer") {
    String subject = str_new("Test string");
    expect(subject != NULL);
    expect(malloc_count == 1);
    str_delete(&subject);
    expect(subject == NULL);
    expect(free_count == 1);
  }

  it("does a no-op when trying to free empty string") {
    String subject = str_empty;
    expect(subject != NULL);
    str_delete(&subject);
    expect(free_count == 0);
    expect(subject == NULL);
  }

  it("does a no-op when trying to free true string") {
    String subject = str_true;
    expect(subject != NULL);
    str_delete(&subject);
    expect(free_count == 0);
    expect(subject == NULL);
  }

  it("does a no-op when trying to free false string") {
    String subject = str_false;
    expect(subject != NULL);
    str_delete(&subject);
    expect(free_count == 0);
    expect(subject == NULL);
  }

  it("does a no-op when trying to free pointer to NULL") {
    String subject = NULL;
    str_delete(&subject);
    expect(free_count == 0);
  }

  it("does a no-op when trying to free NULL") {
    str_delete(NULL);
    expect(free_count == 0);
  }

}

describe(str_eq) {

  it("tests that two strings are bitwise equal") {
    char str_1[] = "String";
    char str_2[] = "String";

    // strings made on the stack shouldn't have the same address
    expect(&str_1[0] != &str_2[0]);

    StringRange range1 = str_range(str_1);
    StringRange range2 = str_range(str_2);

    expect(str_eq(range1, range2));
  }

  it("tests that two strings are not equal") {
    char str_1[] = "String";
    char str_2[] = "StrinG";

    // strings made on the stack shouldn't have the same address
    expect(&str_1[0] != &str_2[0]);

    StringRange range1 = str_range(str_1);
    StringRange range2 = str_range(str_2);

    expect(not str_eq(range1, range2));
  }

}

describe(str_starts_with) {
  StringRange range = R("This is a string");

  it("handles a basic true use case") {
    expect(str_starts_with(range, R("This")));
  }

  it("handles a basic false use case") {
    expect(not str_starts_with(range, R("Thos")));
  }

  it("is case sensitive") {
    expect(not str_starts_with(range, R("THIS")));
  }

  it("returns true given an empty string") {
    expect(str_starts_with(range, R("")));
  }

  it("returns true given the full string") {
    expect(str_starts_with(range, R("This is a string")));
  }

  it("returns false given more than the full string") {
    expect(not str_starts_with(range, R("This is a string with more")));
  }

}

describe(str_ends_with) {
  StringRange range = R("This is a string");

  it("handles a basic true use case") {
    expect(str_ends_with(range, R("string")));
  }

  it("handles a basic false use case") {
    expect(not str_ends_with(range, R("strong")));
  }

  it("is case sensitive") {
    expect(not str_ends_with(range, R("STRING")));
  }

  it("returns true given an empty string") {
    expect(str_ends_with(range, R("")));
  }

  it("returns true given the full string") {
    expect(str_ends_with(range, R("This is a string")));
  }

  it("returns false given more than the full string") {
    expect(not str_ends_with(range, R("And This is a string")));
  }

}

describe(str_contains) {
  StringRange range = R("This is a string");

  it("handles a basic true use case") {
    expect(range to match("is a", str_contains));
  }

  it("handles a basic false use case") {
    expect(range to not match("not in", str_contains));
  }

  it("is case sensitive") {
    expect(range to not match("IS A", str_contains));
  }

  it("returns true given an empty string") {
    expect(range to match("", str_contains));
  }

  it("returns true given the full string") {
    expect(range to match("This is a string", str_contains));
  }

  it("returns false given more than the full string") {
    expect(range to not match("This is a string.", str_contains));
  }

}

describe(str_to_bool) {
  bool out = false, *p_out = &out;

  it("fails when no output parameter is given") {
    p_out = NULL;
    expect(str_to_bool to be_false given(str_true, p_out));
  }

  it("converts from exact strings") {
    expect(str_to_bool to be_true given("true", p_out));
    expect(out to be_true);

    expect(str_to_bool to be_true given("false", p_out));
    expect(out to be_false);

    expect(str_to_bool to be_true given(str_true, p_out));
    expect(out to be_true);

    expect(str_to_bool to be_true given(str_false, p_out));
    expect(out to be_false);
  }

  it("is case insensitive") {
    expect(str_to_bool to be_true given("TRUE", p_out));
    expect(out to be_true);

    expect(str_to_bool to be_true given("FALSE", p_out));
    expect(out to be_false);

    expect(str_to_bool to be_true given("True", p_out));
    expect(out to be_true);

    expect(str_to_bool to be_true given("False", p_out));
    expect(out to be_false);
  }

  it("fails to convert empty or too-short strings") {
    expect(str_to_bool to be_false given(str_empty, p_out));
    expect(str_to_bool to be_false given("tru", p_out));
    expect(str_to_bool to be_false given("fals", p_out));
  }

  it("does not accept leading spaces") {
    expect(str_to_bool to be_false given(" true", p_out));
    expect(str_to_bool to be_false given(" false", p_out));
  }

  it("fails to convert strings with non-matching characters") {
    expect(str_to_bool to be_false given("trub", p_out));
    expect(str_to_bool to be_false given("falze", p_out));
  }

  it("allows additional characters after the matching portion") {
    expect(str_to_bool to be_true given("true stuff", p_out));
    expect(out to be_true);

    expect(str_to_bool to be_true given("false statement", p_out));
    expect(out to be_false);
  }

}

describe(str_to_int) {
  index_s out = 0, *p_out = &out;

  it("fails when no output parameter is given") {
    p_out = NULL;
    expect(str_to_int to be_false given("123235", p_out));
  }

  it("fails when input is empty") {
    expect(str_to_int to be_false given(str_empty, p_out));
  }

  it("converts unsinged numbers") {
    expect(str_to_int to be_true given("0", p_out));
    expect(out, == , 0);

    expect(str_to_int to be_true given("10", p_out));
    expect(out, == , 10);

    expect(str_to_int to be_true given("123401234", p_out));
    expect(out, == , 123401234);
  }

  it("converts signed numbers") {
    expect(str_to_int to be_true given("+0", p_out));
    expect(out, == , 0);

    expect(str_to_int to be_true given("+10", p_out));
    expect(out, == , 10);

    expect(str_to_int to be_true given("+123401234", p_out));
    expect(out, == , 123401234);

    expect(str_to_int to be_true given("-1", p_out));
    expect(out, == , -1);

    expect(str_to_int to be_true given("-10", p_out));
    expect(out, == , -10);

    expect(str_to_int to be_true given("-7482934", p_out));
    expect(out, == , -7482934);
  }

  it("allows trailing characters") {
    expect(str_to_int to be_true given("0 items", p_out));
    expect(out, == , 0);

    expect(str_to_int to be_true given("10 monkeys", p_out));
    expect(out, == , 10);

    expect(str_to_int to be_true given("3985206 is a number", p_out));
    expect(out, == , 3985206);
  }

  it("doesn't allow leading spaces") {
    expect(str_to_int to be_false given(" 5", p_out));
  }

  it("fails with non-numeric input") {
    expect(str_to_int to be_false given("a5", p_out));
  }

}

describe(str_index_of) {
  StringRange range = R("This is a string");
  StringRange is = R("is");

  it("finds an index") {
    expect(str_index_of(range, is, 0), == , 2u);
  }

  it("fails to find a substring that isn't present") {
    expect(str_index_of(range, R("Not present"), 0), == , range.size);
  }

  it("begins the search from a given position") {
    expect(str_index_of(range, is, 3), == , 5u);
  }

  it("fails to find a word in the string that starts before the start_pos") {
    expect(str_index_of(range, is, 6), == , range.size);
  }

  it("fails to find anything if the start pos is after the end of the string") {
    expect(str_index_of(range, is, 99), == , range.size);
  }

  it("looping end-pos can be used to track through the string") {
    index_s tracker = 0;

    tracker = str_index_of(range, "i", tracker);
    StringRange result = str_substring(range, tracker);
    expect(result to match("is is a string", str_eq));

    tracker = str_index_of(range, "i", tracker+1);
    result = str_substring(range, tracker);
    expect(result to match("is a string", str_eq));

    tracker = str_index_of(range, "i", tracker+1);
    result = str_substring(range, tracker);
    expect(result to match("ing", str_eq));

    tracker = str_index_of(range, "i", tracker+1);
    expect(tracker, == , range.size);
  }

}

describe(str_find) {
  StringRange range = R("This is a string");

  it("finds an index") {
    StringRange is = R("is");
    expect(istr_find(range, is), == , 2u);
  }

  it("fails to find a substring that isn't present") {
    StringRange np = R("not present");
    expect(istr_find(range, np), == , range.size);
  }

  it("can find the first word in the string") {
    StringRange This = R("This");
    expect(istr_find(range, This), == , 0u);
  }

  expect(malloc_count == 0);

}

describe(str_substring) {

  StringRange range = R("This is a string");

  context("with only ranges, no allocations") {

    it("gets a substring from 0 to 0") {
      StringRange subject = str_substring(range, 0, 0);
      expect(subject to match(str_empty, str_eq));
    }

    it("gets substring of whole string (using default argument") {
      StringRange subject = str_substring(range, 0);
      expect(subject to match(range, str_eq));
    }

    it("gets a partial substring from the beginning") {
      StringRange subject = str_substring(range, 0, 4);
      expect(subject to match("This", str_eq));
    }

    it("gets a substring starting partway in the string") {
      StringRange subject = str_substring(range, 5, 9);
      expect(subject to match("is a", str_eq));
    }

    it("uses a negative offset for the start of the substring") {
      StringRange subject = str_substring(range, -6);
      expect(subject to match("string", str_eq));
    }

    it("uses negative offsets for the start and end") {
      StringRange subject = str_substring(range, -8, -3);
      expect(subject to match("a str", str_eq));
    }

    it("has a string start past the end") {
      StringRange subject = str_substring(range, 20);
      expect(subject to match(str_empty, str_eq));
    }

    it("has a string end before the beginning") {
      StringRange subject = str_substring(range, 5, 3);
      expect(subject to match(str_empty, str_eq));
    }

    it("has a string end before the beginning") {
      StringRange subject = str_substring(range, 1, 0);
      expect(subject to match(str_empty, str_eq));
    }

    it("can accept a basic c-string") {
      StringRange subject = str_substring("This is a string", -6);
      expect(subject to match("string", str_eq));
    }

    expect(malloc_count == 0);

  }

  it("can accept a dynamic String") {
    String str = str_copy(range);
    StringRange subject = str_substring(str, -6);
    expect(subject to match("string", str_eq));
    str_delete(&str);
  }

}

describe(str_trim) {

  StringRange range = R("\t  String with extra spaces   \n");
  StringRange subject = str_empty->range;
  StringRange expected = str_empty->range;

  it("trims all leading and trailing spaces") {
    subject = str_trim(range);
    expected = R("String with extra spaces");
  }

  it("trims all leading spaces") {
    subject = str_trim_start(range);
    expected = R("String with extra spaces   \n");
  }

  it("trims all trailing spaces") {
    subject = str_trim_end(range);
    expected = R("\t  String with extra spaces");
  }

  it("Does nothing to a string with no leading or trailing spaces") {
    subject = str_trim(str_true);
    expected = R("true");
  }

  after{
    expect(subject to match(expected, str_eq));
    expect(malloc_count == 0);
  }

}

describe(str_split) {

  StringRange range = R("This is, a collection, of strings");
  Array_StringRange result = NULL;

  it("performs a basic split on commas") {
    result = str_split(range, ",");
    expect(result->size, == , 3u);

    StringRange expected[3] = { R("This is"), R(" a collection"), R(" of strings") };
    expect(result to all(str_eq, expected[n], StringRange, array));
  }

  it("performs a multi-char split") {
    result = str_split(range, ", ");
    expect(result->size, == , 3u);

    StringRange expected[3] = { R("This is"), R("a collection"), R("of strings") };
    expect(result to all(str_eq, expected[n], StringRange, array));
  }

  it("splits on an empty string") {
    result = str_split(range, str_empty->range);
    expect(result->size, == , range.size);

    expect(result to all(1 == str_size, StringRange, array));
    expect(result to all(str_eq, str_substring(range, n, n+1), StringRange, array));
  }

  it("tries to split on a delimiter that isn't present") {
    result = str_split(range, "NOT_INCLUDED");
    expect(result->size, == , 1u);
    expect(result to all(str_eq, range, StringRange, array));
  }

  it("tries to split with a delimiter that is too big") {
    result = str_split(range, "This is, a collection, of strings, but more");
    expect(result->size, == , 1u);
    expect(result to all(str_eq, range, StringRange, array));
  }

  it("splits using the same string as the delimiter") {
    result = str_split(range, range);
    expect(result->size, == , 2u);
    expect(result to all(str_eq, str_empty->range, StringRange, array));
  }

  if (result) {
    arr_str_delete(&result);
  }

}

describe(str_join) {

  Array_StringRange tokens = NULL;
  String result = NULL;

  context("basic set of StringRange tokens to form a sentence") {

    StringRange range = R("These are the test tokens");
    tokens = str_split(range, " ");

    it("recreates the original string") {
      result = str_join(" ", tokens);
      expect(result to match(range, str_eq));
    }

    it("puts together the string without spaces ") {
      result = str_join("", tokens);
      expect(result to match("Thesearethetesttokens", str_eq));
    }

    it("gives a multi-char deliminiter between the tokens") {
      result = str_join(" - ", tokens);
      expect(result to match("These - are - the - test - tokens", str_eq));
    }

  }

  context("given an empty array of String") {

    tokens = arr_str_new_reserve(0);

    it("produces an empty string") {
      result = str_join("!", tokens);
      expect(result->size, == , 0u);
      expect(result == str_empty);
    }

  }

  context("with an array of dynamic String objects") {

    String str1 = str_new("Str 1");
    String str2 = str_new("Str 2");
    String str3 = str_new("Str 3");
    tokens = arr_str_new_reserve(3);
    arr_str_push_back(tokens, str1->range);
    arr_str_push_back(tokens, str2->range);
    arr_str_push_back(tokens, str3->range);

    it("properly joins the strings") {
      result = str_join(", ", tokens);
      expect(result to match("Str 1, Str 2, Str 3", str_eq));
    }

    it("can mix String and StringRange* in the same array") {
      StringRange range = R("Range 4");
      arr_str_push_back(tokens, range);

      result = str_join("|", tokens);

      expect(result to match("Str 1|Str 2|Str 3|Range 4", str_eq));
    }

    str_delete(&str1);
    str_delete(&str2);
    str_delete(&str3);
  }

  if (result) str_delete(&result);
  if (tokens) arr_str_delete(&tokens);

}

describe(str_concat) {
  String result = NULL;

  it("joins two strings together") {
    result = str_concat("LHS ", "RHS");
    expect(result to match("LHS RHS", str_eq));
  }

  it("still copies the string if joining with an empty (rhs)") {
    char* lhs = "LHS";
    result = str_concat(lhs, str_empty);
    expect(result to match("LHS", str_eq));
    expect(result->begin, != , lhs);
  }

  it("still copies the string if joining with an empty (lhs)") {
    char* rhs = "RHS";
    result = str_concat(str_empty, rhs);
    expect(result to match("RHS", str_eq));
    expect(result->begin, != , rhs);
  }

  it("makes an empty string when both are empty") {
    result = str_concat(str_empty, str_empty);
    expect(result to match(str_empty, str_eq));
    expect(result, == , str_empty);
  }

  if (result) str_delete(&result);

}

describe(str_format) {
  String result = NULL;

  context("basic formatter cases") {

    it("returns a copy when no format specifiers are present") {
      StringRange test = R("Passthrough");
      result = str_format(test, "unused");
      expect(result to match(test, str_eq));
      expect(result->begin != test.begin);
    }

    it("escapes an open brace") {
      result = str_format("{{}", "unused");
      expect(result to match("{}", str_eq));
    }

    it("does a basic string replacement") {
      result = str_format("{} Blah", "Replacement Second");
      expect(result to match("Replacement Second Blah", str_eq));
    }

    it("directly prints any invalid format specifier") {
      result = str_format("|{}|{a}|", "first", "unused");
      expect(result to match("|first|{a}|", str_eq));
    }

    it("won't try to substitute based on braces in arguments") {
      result = str_format("{}", "{}", "unused");
      expect(result to match("{}", str_eq));
    }

    it("does not allow spaces in a format specifier") {
      result = str_format("{ }", "unused");
      expect(result to match("{ }", str_eq));
    }

    it("can output a basic json without formatting it") {
      result = str_format("{json_param: 2, obj: {a: 1}}", "unused");
      expect(result to match("{json_param: 2, obj: {a: 1}}", str_eq));
    }

    it("does a substitution with two format specifiers in sequence") {
      result = str_format("1: {}, 2: {}", "first", "second");
      expect(result to match("1: first, 2: second", str_eq));
    }

    it("treats arg indexes past list size as blank") {
      result = str_format("|{}|{}|", "test");
      expect(result to match("|test||", str_eq));
    }

    it("completely pads an argument out of range") {
      result = str_format("|{1:5}|", "unused");
      expect(result to match("|     |", str_eq));
    }

  }

  context("positional arguments") {

    it("indexes the arg list when given a formatter index") {
      result = str_format("|{0}|{0}|{0}|", "test");
      expect(result to match("|test|test|test|", str_eq));
    }

    it("can index the arg list in arbitrary order") {
      result = str_format("|{1}|{0}|{2}|{0}|", "first", "second", "third");
      expect(result to match("|second|first|third|first|", str_eq));
    }

    it("treats explicitly provided out-of-range index as blank") {
      result = str_format("|{1}|", "only");
      expect(result to match("||", str_eq));
    }

    it("continues counting arguments after the first explicit index") {
      result = str_format("|{1}|{}|", "first", "second", "third");
      expect(result to match("|second|third|", str_eq));
    }

  }

  context("alignment and padding") {

    it("can set a minimum width for an argument") {
      result = str_format("|{:10}|", "test");
      expect(result to match("|test      |", str_eq));
    }

    it("can take an alignment specifier (left default)") {
      result = str_format("|{:<10}|", "test");
      expect(result to match("|test      |", str_eq));
    }

    it("can take an alignment specifier (right)") {
      result = str_format("|{:>10}|", "test");
      expect(result to match("|      test|", str_eq));
    }

    it("can take an alignment specifier (center)") {
      result = str_format("|{:^10}|", "test");
      expect(result to match("|   test   |", str_eq));
    }

    it("can handle an odd width center alignment") {
      result = str_format("|{:^9}|", "test");
      expect(result to match("|   test  |", str_eq));
    }

    it("uses a given char for padding") {
      result = str_format("|{:#-10}|", "test");
      expect(result to match("|test------|", str_eq));
    }

    it("combines alignment and padding char") {
      result = str_format("|{:>#-10}|", "test");
      expect(result to match("|------test|", str_eq));
    }

    it("allows combining of positional arguments and alignments") {
      result = str_format("|{1:>#`5}|{0:<#,5}|", "a", "b");
      expect(result to match("|````b|a,,,,|", str_eq));
    }

  }

  context("formatting for integer values") {

    it("prints an integer with no special formatting") {
      result = str_format("{}", 15);
      expect(result to match("15", str_eq));
    }

    it("can output zero") {
      result = str_format("{}", 0);
      expect(result to match("0", str_eq));
    }

    context("sign display") {

      it("shows + sign on positive numbers") {
        result = str_format("{:+}", 15);
        expect(result to match("+15", str_eq));
      }

      it("positive zero") {
        result = str_format("{:+}", 0);
        expect(result to match("+0", str_eq));
      }

      it("negative zero") {
        result = str_format("{:+}", -0);
        expect(result to match("+0", str_eq));
      }

      it("prints an integer with no special formatting") {
        result = str_format("{}", -15);
        expect(result to match("-15", str_eq));
      }

      it("prints a negative integer with explicit sign") {
        result = str_format("{:+}", -15);
        expect(result to match("-15", str_eq));
      }

    }

    context("alignment and padding") {

      it("prints a left aligned integer with padding") {
        result = str_format("|{:10}|", 15);
        expect(result to match("|15        |", str_eq));
      }

      it("prints an explicitly left aligned integer with padding") {
        result = str_format("|{:<10}|", 15);
        expect(result to match("|15        |", str_eq));
      }

      it("prints a right aligned integer with padding") {
        result = str_format("|{:>10}|", 15);
        expect(result to match("|        15|", str_eq));
      }

      it("prints a right aligned integer with padding and sign") {
        result = str_format("|{:+>10}|", 15);
        expect(result to match("|       +15|", str_eq));
      }

      it("prints a value with ledger alignment") {
        result = str_format("|{:+=10}|", 15);
        expect(result to match("|+       15|", str_eq));
      }

      it("prints a value with ledger alignment sans plus") {
        result = str_format("|{:=10}|", 15);
        expect(result to match("|        15|", str_eq));
      }

      it("prints a negative value with ledger alignment") {
        result = str_format("|{:=10}|", -3457);
        expect(result to match("|-     3457|", str_eq));
      }

      it("prints with leading zeros") {
        result = str_format("|{:010}|", 832983);
        expect(result to match("|0000832983|", str_eq));
      }

      it("prints with explicitly defined padding character") {
        result = str_format("|{:#.010}|", -345871);
        expect(result to match("|-...345871|", str_eq));
      }

      it("prints a number centered") {
        result = str_format("|{:^10}|", 124);
        expect(result to match("|    124   |", str_eq));
      }

      it("prints a signed number centered") {
        result = str_format("|{:^+10}|", 124);
        expect(result to match("|   +124   |", str_eq));
      }

      it("prints a negative number centered") {
        result = str_format("|{:^10}|", -1246);
        expect(result to match("|   -1246  |", str_eq));
      }

      it("prints middle zero") {
        result = str_format("({:^9})", 0);
        expect(result to match("(    0    )", str_eq));
      }

      it("prints a number larger than the given width") {
        result = str_format("|{:5}|", 1234567);
        expect(result to match("|1234567|", str_eq));
      }

    }

    context("representation styles") {

      it("prints a regular number") {
        result = str_format("{}", 65);
        expect(result to match("65", str_eq));
      }

      it("prints a hex number") {
        result = str_format("{!x}", 65);
        expect(result to match("41", str_eq));
      }

      it("prints hex zero") {
        result = str_format("{!x}", 0);
        expect(result to match("0", str_eq));
      }

      it("prints a hex number containing letters") {
        result = str_format("{!x}", 64206);
        expect(result to match("face", str_eq));
      }

      it("prints a large mixed hex number") {
        result = str_format("{!x}", 2048397);
        expect(result to match("1f418d", str_eq));
      }

      it("prints capitalized hex letters") {
        result = str_format("{!X}", 64206);
        expect(result to match("FACE", str_eq));
      }

      it("prints using binary") {
        result = str_format("{!b:08}", 109);
        expect(result to match("01101101", str_eq));
      }

      it("prints a character") {
        result = str_format("{!c}", 65);
        expect(result to match("A", str_eq));
      }

      it("prints a character with padding") {
        result = str_format("{!c:^7}", 65);
        expect(result to match("   A   ", str_eq));
      }

    }

  }

  context("formatting for floating point values") {

    it("prints a float with default formatting") {
      result = str_format("{}", 123.456);
      expect(result to match("123.4", str_eq));
    }

    it("prints a float zero") {
      result = str_format("{}", 0.0);
      expect(result to match("0", str_eq));
    }

    it("doesn't include a decimal for whole numbers") {
      result = str_format("{}", 234.0);
      expect(result to match("234", str_eq));
    }

    it("can print negative numbers") {
      result = str_format("{}", -34.28);
      expect(result to match("-34.2", str_eq));
    }

    context("precision controls") {

      it("goes beyond a precision level of 1") {
        result = str_format("{:.2}", 123.456);
        expect(result to match("123.45", str_eq));
      }

      it("goes beyond a precision level of 2") {
        result = str_format("{:.05}", 123.456);
        expect(result to match("123.456", str_eq));
      }

      it("prints a double with fixed precision and trailing zeroes") {
        result = str_format("|{:+10.3+}|", 5.4);
        expect(result to match("|+5.400    |", str_eq));
      }

    }

    context("alignment and padding") {

      it("prints a left aligned double with padding") {
        result = str_format("|{:10}|", 15.54321);
        expect(result to match("|15.5      |", str_eq));
      }

      it("prints a left aligned double with higher precision with padding") {
        result = str_format("|{:10.3}|", 15.54321);
        expect(result to match("|15.543    |", str_eq));
      }

      it("prints a right aligned double") {
        result = str_format("|{:>10.3}|", 15.54321);
        expect(result to match("|    15.543|", str_eq));
      }

      it("prints a right aligned double with sign") {
        result = str_format("|{:>+10.3}|", 15.54321);
        expect(result to match("|   +15.543|", str_eq));
      }

      it("prints a ledger aligned double with sign") {
        result = str_format("|{:=+10.3}|", 15.54321);
        expect(result to match("|+   15.543|", str_eq));
      }

      it("prints a ledger aligned double with negative value") {
        result = str_format("|{:=+10.3}|", -15.54321);
        expect(result to match("|-   15.543|", str_eq));
      }

      it("prints a double with fixed precision") {
        result = str_format("|{:+10.5}|", 5.4);
        expect(result to match("|+5.4      |", str_eq));
      }

      it("prints a double with fixed precision and trailing zeroes") {
        result = str_format("|{:+10.5+}|", 5.4);
        expect(result to match("|+5.40000  |", str_eq));
      }

      it("prints ledger-aligned with always 2 decimals") {
        result = str_format("|{:=+10.2+}|", 23.634);
        expect(result to match("|+    23.63|", str_eq));
      }

      it("prints ledger-aligned with always 2 decimals, given a whole number") {
        result = str_format("|{:=+10.2+}|", 23.0);
        expect(result to match("|+    23.00|", str_eq));
      }

      it("prints ledger-aligned with trailing zeroes and zero-fill") {
        result = str_format("|{:+010.2+}|", 23.0);
        expect(result to match("|+000023.00|", str_eq));
      }

    }

  }

  //if (result) test_log_memory(result->begin);
  if (result) str_delete(&result);

}

test_suite(tests_string) {
  test_group(str_range),
  test_group(str_new),
  test_group(str_copy),
  test_group(str_from_bool),
  test_group(str_from_int),
  test_group(str_from_float),
  test_group(str_delete),
  test_group(str_eq),
  test_group(str_starts_with),
  test_group(str_ends_with),
  test_group(str_contains),
  test_group(str_to_bool),
  test_group(str_to_int),
  test_group(str_index_of),
  test_group(str_find),
  test_group(str_substring),
  test_group(str_trim),
  test_group(str_split),
  test_group(str_join),
  test_group(str_concat),
  test_group(str_format),
  test_suite_end
};

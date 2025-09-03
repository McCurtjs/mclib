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

#define CSPEC_CUSTOM_TYPES  \
  slice_t: "slice_t",       \
  slice_t*: "slice_t*",     \
  String: "String"          //

#include "cspec.h"

describe(str_build) {
  String subject = NULL;

  char* c_str = "This is a c-string";

  it("makes a copy using the _s version") {
    subject = str_build(c_str, 4);
    expect(subject to match("This", str_eq));
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
  slice_t slice = S(c_str);

  it("allocates a new copy of the string directly from the c string") {
    subject = str_copy(c_str);
    expect(subject to match(p_str, str_eq));
    expect(subject->begin != c_str);
  }

  it("allocates a new copy of the string from a slice") {
    subject = str_copy(slice);
    expect(subject to match(slice, str_eq));
    expect(subject->begin != slice.begin);
  }

  it("allocates a new copy of the string from another dynamic string") {
    String str = str_copy(c_str);
    subject = str_copy(str);
    expect(subject to match(str, str_eq));
    str_delete(&str);
  }

  it("ensures the direct and range pointers are the same/shared in union") {
    subject = str_copy(c_str);
    expect(&subject->begin == &subject->slice.begin);
    expect(&subject->size == &subject->slice.size);
    expect(&subject->size == &subject->length);
  }

  if (subject) {
    expect(subject->begin != slice.begin);
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
    expect(str_eq(subject->slice, S("0")));
  }

  it("gets pretty exact values from whole numbers") {
    subject = str_from_float(4.0f);
    expect(str_eq(subject->slice, S("4")));
  }

  it("gets pretty exact values from negative whole numbers") {
    subject = str_from_float(-9.0f);
    expect(str_eq(subject->slice, S("-9")));
  }

  it("gets decimal values") {
    subject = str_from_float(2.73f);
    expect(subject to match(S("2.73"), str_eq));
  }

  if (subject) {
    str_delete(&subject);
  }

}

describe(str_delete) {

  it("frees the memory and zeroes the pointer") {
    String subject = str_copy("Test string");
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

describe(str_substring) {

  slice_t slice = S("Substring test");

  it("resolves the macro correctly") {
    slice_t subject = str_substring(slice, 3);
    expect(subject to match("string test", str_eq));
  }
}

describe(str_join) {

  Array_slice tokens = NULL;
  String result = NULL;

  context("basic set of slice_t tokens to form a sentence") {

    slice_t range = S("These are the test tokens");
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

    tokens = arr_slice_new_reserve(0);

    it("produces an empty string") {
      result = str_join("!", tokens);
      expect(result->size, == , 0u);
      expect(result == str_empty);
    }

  }

  context("with an array of dynamic String objects") {

    String str1 = str_copy("Str 1");
    String str2 = str_copy("Str 2");
    String str3 = str_copy("Str 3");
    tokens = arr_slice_new_reserve(3);
    arr_slice_push_back(tokens, str1->slice);
    arr_slice_push_back(tokens, str2->slice);
    arr_slice_push_back(tokens, str3->slice);

    it("properly joins the strings") {
      result = str_join(", ", tokens);
      expect(result to match("Str 1, Str 2, Str 3", str_eq));
    }

    it("can mix String and slice_t* in the same array") {
      slice_t range = S("Range 4");
      arr_slice_push_back(tokens, range);

      result = str_join("|", tokens);

      expect(result to match("Str 1|Str 2|Str 3|Range 4", str_eq));
    }

    str_delete(&str1);
    str_delete(&str2);
    str_delete(&str3);
  }

  if (result) str_delete(&result);
  if (tokens) arr_slice_delete(&tokens);

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
      slice_t test = S("Passthrough");
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
  test_group(str_build),
  test_group(str_copy),
  test_group(str_from_bool),
  test_group(str_from_int),
  test_group(str_from_float),
  test_group(str_delete),
  test_group(str_substring),
  test_group(str_join),
  test_group(str_concat),
  test_group(str_format),
  test_suite_end
};

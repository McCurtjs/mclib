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

#include "cspec.h"

#include "str.h"

/* wait, does it not include // comments? */

csBool resolve_types(const char** p_type, const void* value) {
  slice_t type = slice_from_c_str(*p_type);
  const slice_t* value_slice = NULL;

  if (str_eq(type, "slice_t*")) {
    value_slice = *(const slice_t**)value;
  }
  else if (str_eq(type, "slice_t")) {
    value_slice = (const slice_t*)value;
  }
  else if (str_eq(type, "String")) {
    value_slice = &(*(const String*)value)->slice;
  }

  if (value_slice) {
    cspec_out_slice(value_slice->begin, (csSize)value_slice->size);
    return TRUE;
  }

  return FALSE;
}

// Basic output function for CSPEC printing

//int puts(const char* s);
#include <stdio.h>

void printer(const char* str, csUint len, csUint color) {
  (void)len;
  (void)color;
  puts(str);
}

// Test suites

extern TestSuite tests_cspec;
extern TestSuite tests_slice;
extern TestSuite tests_string;
extern TestSuite tests_span;
extern TestSuite tests_map;

// Main

int main(int argc, char* argv[]) {

#ifdef CSPEC_MSVC
  /* Test values for Visual Studio without having to modify properties */
  argv = (char* []){ argv[0], "-y", "slice_spec.c:678" };
  argc = 3;
#endif

  TestSuite* test_suites[] = {
    &tests_cspec,
    &tests_slice,
    &tests_string,
    &tests_span,
    &tests_map,
  };

  cspec_opt_print_line = printer;
  cspec_opt_resolve_user_types = resolve_types;
  cspec_opt_print_backtrace = cspec_default_print_backtrace;

  return cspec_run_all(test_suites);
}

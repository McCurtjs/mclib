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

#include "slice.h"
#include "str.h"

csUint resolve_types(
  const char** p_type, const void* value, char* out_buffer, csUint out_size
) {
  slice_t type = slice_build(*p_type);
  const slice_t* value_slice = NULL;
  index_t out_size_s = (index_t)out_size;

  if (slice_eq(type, S("slice_t*"))) {
    value_slice = *(const slice_t**)value;
  }
  else if (str_eq(type, "slice_t")) {
    value_slice = (const slice_t*)value;
  }
  else if (str_eq(type, "String")) {
    value_slice = &(*(const String*)value)->slice;
  }

  if (value_slice) {
    index_s w = 0;
    if (str_ends_with(type, "*") && w < out_size_s) out_buffer[w++] = '&';
    out_buffer[w++] = '"';
    for (index_s i = 0; w < out_size_s - 1 && i < value_slice->size; ++i) {
      out_buffer[w++] = value_slice->begin[i];
    }
    out_buffer[w++] = '"';

    return (csUint)w;
  }

  return 0;
}

// Test suites

extern TestSuite tests_cspec;
extern TestSuite tests_slice;
extern TestSuite tests_string;
extern TestSuite tests_span;

// Main

int main(int argc, char* argv[]) {
  TestSuite* test_suites[] = {
    &tests_cspec,
    &tests_slice,
    &tests_string,
    &tests_span,
  };

  resolve_user_types = resolve_types;

  return cspec_run_all(test_suites);
}

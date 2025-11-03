/*******************************************************************************
* MIT License
*
* Copyright (c) 2025 Curtis McCoy
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

#include "mat.h"
#include "str.h"
#include "span_byte.h"
#include "array_byte.h"
#include "array_slice.h"
#include "map.h"

#include <string.h>

int main(void) {

  vec2i i2 = { 2, 4 };
  vec3i i3 = { 1, 2, 3 };
  vec3b b3 = { 0x11, 0x45, 0xB4 };
  vec4b b4 = { 6, 5, 4, 3 };
  vec2 f2 = { 1.1f, 2.3f };
  vec3 f3 = { 2.3f, 4.5f, 6.7f };
  vec4 f4 = { 1.1f, 2.2f, 3.3f, 4.4f };

  mat4 id = m4identity;

  UNUSED(i2); UNUSED(i3); UNUSED(b3); UNUSED(b4);
  UNUSED(f2); UNUSED(f3); UNUSED(f4); UNUSED(id);

  slice_t slice = S("This is a slice");
  slice_t chop = slice_substring(slice, 0, 4);
  slice_empty;
  slice_true;
  String str = str_copy("This is a string");
  String empty_str = str_empty;
  String null_str = NULL;

  int ints[] = { 1, 2, 3, 4, 5 };
  byte bytes[] = { 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0 };
  slice_t slices[] = { S("First span"), S("second"), slice_substring(slice, 5, 9) };
  span_t span = SPAN(ints);
  span_slice_t splice = SPAN(slices);
  span_byte_t bspan = SPAN(bytes);

  Array arr = arr_new_reserve(int, 5);
  arr_write_back(arr, ints);
  arr_write_back(arr, &(int){ 0xFFFFFFFF });
  arr_write_back(arr, &(int){ 0xAAAAAAAA });

  array_t local_array = arr_build(int);
  arr_write_back(&local_array, &ints[3]);
  arr_free(&local_array);

  array_byte_t asdf = arr_byte_build_reserve(5);
  arr_byte_write_back(&asdf, (byte*)&(int){ 65 });
  arr_byte_write_back(&asdf, (byte*)&(int){ 66 });
  arr_byte_write_back(&asdf, (byte*)&(int){ 67 });
  array_slice_t oisjdg = arr_slice_build();
  arr_slice_write_back(&oisjdg, &slices[2]);
  arr_slice_write_back(&oisjdg, &slices[2]);
  arr_slice_write_back(&oisjdg, &slices[2]);
  arr_slice_write_back(&oisjdg, &slices[2]);
  arr_slice_write_back(&oisjdg, &slices[2]);

  Array arrv2 = arr_new(vec2);
  arr_write_back(arrv2, &v2zero);
  arr_write_back(arrv2, &v2ones);

  Array_byte arrb = arr_byte_new_reserve(8);
  memcpy(arr_byte_emplace_back_range(arrb, sizeof(bytes)).begin, bytes, sizeof(bytes));

  Array_slice arrs = arr_slice_new();
  arr_slice_push_back(arrs, slices[1]);
  arr_slice_push_back(arrs, slices[0]);
  arr_slice_push_back(arrs, chop);

  HMap map = map_new(int, int, NULL, NULL);
  map_write(map, &(int) { 5 }, & (int) { 12 });
  map_write(map, &(int) { 122 }, & (int) { 512432145 });
  map_write(map, &(int) { 1337 }, & (int) { 12 });

  __debugbreak();

  return 0;
}






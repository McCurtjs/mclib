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

#include "arena.h"

#define CSPEC_CUSTOM_TYPES \
  Arena: "Arena"

#include "cspec.h"

describe(arena_stuff) {

  it("makes an arena") {
    Arena arena = arena_new_reserve(1024);
    expect(arena to not be_null);

    int* memory = arena_alloc(arena, sizeof(int) * 3);

    for (int i = 0; i < 3; ++i) {
      memory[i] = i;
    }

    expect(arena->bytes_used == sizeof(int) * 3);

    arena_delete(&arena);
    expect(arena to be_null);
  }

}

test_suite(tests_arena) {
  test_group(arena_stuff),
  test_suite_end
};

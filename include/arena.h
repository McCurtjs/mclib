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

#ifndef MCLIB_ARENA_H_
#define MCLIB_ARENA_H_

#include "types.h"

typedef struct _opaque_Arena_t {
  index_t CONST page_count;
  index_t CONST alloc_count;
  index_t CONST bytes_used;
}* Arena;

Arena arena_new(void);
Arena arena_new_reserve(index_t capacity);
void  arena_reserve(Arena, index_t capacity);
void  arena_clear(Arena);
void  arena_free(Arena);
void  arena_delete(Arena*);

void* arena_alloc(Arena, index_t size_bytes);
void* arena_alloc_zeroed(Arena, index_t size_bytes);

#endif

/*

Ideas for Arena allocator:

arena_t or Arena stores block of memory (either allocated on the heap, or declared on the stack).

struct arena_t {
  byte* base; // null for malloc
  index_t size;
  struct arena_t* prev;
  bool is_auto;
};

Arenas operate as a stack, making a new one takes over allocations until released. If "base" is
    null, use default malloc. Previous allocator contains current, unless "default" is re-added
    onto the stack.

If an arena runs out of space, instead of failing, a new arena could be added to the stack with
    the "is_auto" flag set. When popping the stack to remove an allocator, if the flag is set
    keep popping until the top of the stack has is_auto == false (so it functions as if it was
    just one allocator).
Add uint max_autos that sets how many auto allocators can be created? (decrement by one each time
    another is added to the stack).

*/

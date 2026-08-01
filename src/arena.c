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

#define MCLIB_INTERNAL_IMPL
#include "arena.h"

#include <stdlib.h>
#include <string.h.>

typedef struct page_header_t {
  struct page_header_t* next;
  index_t remaining;
  byte data[];
} page_header_t;

#define PAGE_SIZE (4096 - sizeof(page_header_t))

typedef struct Arena_Internal {
  struct _opaque_Arena_t pub;
  page_header_t* pages;
  byte* head;
} Arena_Internal;

void _check_add_page(Arena_Internal* arena, index_t size) {
  if (arena->pages && arena->pages->remaining >= size) return;

  size_t page_size = MAX((size_t)size, PAGE_SIZE);

  page_header_t* page = malloc(page_size + sizeof(page_header_t));
  assert(page);

  *page = (page_header_t) {
    .next = arena->pages,
    .remaining = page_size,
  };

  arena->pages = page;
  arena->head = page->data;
  arena->pub.page_count += 1;
}

Arena arena_new(void) {
  Arena_Internal* arena = malloc(sizeof(Arena_Internal));
  assert(arena);

  *arena = (Arena_Internal) {
    .pub = { 0 },
    .pages = NULL,
    .head = NULL,
  };

  return (Arena)arena;
}

Arena arena_new_reserve(index_t capacity) {
  assert(capacity > 0);
  Arena arena = arena_new();
  arena_reserve(arena, capacity);
  return arena;
}

void arena_reserve(Arena a_in, index_t capacity) {
  assert(capacity > 0);
  assert(a_in);
  Arena_Internal* arena = (Arena_Internal*)a_in;

  if (arena->pages && arena->pages->remaining >= capacity) return;

  page_header_t* page = malloc((size_t)capacity + sizeof(page_header_t));
  assert(page);

  *page = (page_header_t){
    .next = arena->pages,
    .remaining = capacity,
  };

  arena->pages = page;
  arena->head = page->data;
  arena->pub.page_count += 1;
}

void arena_free(Arena a_in) {
  Arena_Internal* arena = (Arena_Internal*)a_in;
  assert(arena);

  page_header_t* page = arena->pages;
  while (page) {
    page_header_t* next = arena->pages->next;
    free(page);
    page = next;
  }

  *arena = (Arena_Internal){
    .pub = {
      .page_count = 0,
      .alloc_count = 0,
      .bytes_used = 0,
    },
    .pages = NULL,
    .head = NULL,
  };
}

void arena_delete(Arena* p_arena) {
  if (!p_arena || !*p_arena) return;
  Arena arena = *p_arena;
  arena_free(arena);
  free(arena);
  *p_arena = NULL;
  return;
}

void* arena_alloc(Arena a_in, index_t size_bytes) {
  assert(a_in);
  assert(size_bytes >= 0);
  Arena_Internal* arena = (Arena_Internal*)a_in;
  if (size_bytes == 0) return NULL;
  _check_add_page(arena, size_bytes);
  arena->pages->remaining -= size_bytes;
  void* ret = arena->head;
  arena->head += size_bytes;
  arena->pub.bytes_used += size_bytes;
  arena->pub.alloc_count += 1;
  return ret;
}

void* arena_alloc_zeroed(Arena a_in, index_t size_bytes) {
  void* ret = arena_alloc(a_in, size_bytes);
  memset(ret, 0, (size_t)size_bytes);
  return ret;
}

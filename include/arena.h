

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

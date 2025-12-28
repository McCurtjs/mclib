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

#ifndef MCLIB_SLOTKEY_H_
#define MCLIB_SLOTKEY_H_

#include "types.h"

typedef struct slotkey_t {
  uint64_t hash;
} slotkey_t;

#define SK_NULL (slotkey_t) { 0 }
#define SK_INDEX_BITS 24
#define SK_UNIQUE_BITS 40
#define SK_INDEX_MASK ((1ull << SK_INDEX_BITS) - 1ull)
#define SK_INDEX_MAX ((1l << SK_INDEX_BITS) - 1l)
#define SK_UNIQUE_MAX ((1ull << SK_UNIQUE_BITS) - 1ull)

static inline int32_t sk_index(slotkey_t key) {
  return key.hash & SK_INDEX_MASK;
}

static inline uint64_t sk_unique(slotkey_t key) {
  return key.hash >> SK_INDEX_BITS;
}

static inline slotkey_t sk_build(index_t index, uint64_t unique) {
  return (slotkey_t) {
    .hash = (unique << SK_INDEX_BITS) | (index & SK_INDEX_MASK)
  };
}

#endif

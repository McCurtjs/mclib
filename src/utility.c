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

#include "types.h"

#include <stdlib.h>
#include <string.h>

#include "murmur3.h"

// WASI doesn't support stof yet, which is annoying. Too lazy to make a function
// right now, pulled from Karl Knechtel at:
// https://stackoverflow.com/questions/4392665/converting-string-to-float-without-stof-in-c
float stof(const char* s) {
  float rez = 0, fact = 1;
  if (*s == '-'){
    s++;
    fact = -1;
  };
  for (int point_seen = 0; *s; s++){
    if (*s == '.'){
      point_seen = 1;
      continue;
    };
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      if (point_seen) fact /= 10.0f;
      rez = rez * 10.0f + (float)d;
    };
  };
  return rez * fact;
}

int stoi(const char* s) {
  return atoi(s);
}

#define NUM_BUF_SIZE 20
static char number_result[NUM_BUF_SIZE + 1];

const char* itos(int i) {
  char* c = number_result;
  char* r = c;

  if (i < 0) {
    *c++ = '-';
    i *= -1;
    ++r;
  }

  while (i) {
    *(c++) = '0' + i % 10;
    i /= 10;
  }

  if (c == number_result) {
    *c++ = '0';
  }

  *c-- = '\0';

  while (r < c) {
    i = *r;
    *r++ = *c;
    *c-- = (char)i;
  }

  return number_result;
}

static char float_result[NUM_BUF_SIZE + 1];

#define FLOAT_PRECISION 100000
const char* ftos(float f) {
  uint i = 0;
  const char* integer_part = itos((int)f);
  if (f < 0) f *= -1;

  while (integer_part[i]) {
    float_result[i] = integer_part[i];
    ++i;
  }
  float_result[i] = '\0';

  uint decimal = (uint)(f * FLOAT_PRECISION) % FLOAT_PRECISION;
  if (!decimal) return float_result;

  float_result[i++] = '.';

  const char* decimal_part = itos(decimal);
  uint j = 0;

  while (decimal_part[j]) {
    float_result[i++] = decimal_part[j++];
  }

  do {
    float_result[i--] = '\0';
  } while (float_result[i] == '0');

  return float_result;
}

void memrev(void* p, unsigned size) {
  byte* s = p;
  byte* e = s + size - 1;
  while (s < e) {
    byte t = *s;
    *s++ = *e;
    *e-- = t;
  }
}

// Most significant bit algorithm, from:
//    https://aggregate.org/MAGIC/#Most%20Significant%201%20Bit
size_t msb(size_t x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
#ifdef MCLIB_64
  x |= (x >> 32);
#endif
  return(x & ~(x >> 1));
}

hash_t hash(const void* src, index_t size) {
  hash_t ret;
#ifdef MCLIB_64
  byte output[16];
  MurmurHash3_x64_128(src, (int)size, 0, &output);
  ret = *(hash_t*)output;
#else
  MurmurHash3_x86_32(src, (int)size, 0, &ret);
#endif
  return ret;
}

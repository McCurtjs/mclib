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

#ifndef MCLIB_QUAT_H_
#define MCLIB_QUAT_H_

#include "types.h"

#include "vec.h"
#include "mat.h"

#define q4floats    4
#define q4bytes     (sizeof(quat))

#define q4zero      ((quat){.f={ 0, 0, 0, 0 } })
#define q4identity  ((quat){.f={ 0, 0, 0, 1 } })

#define I static inline

I float q4magsq(quat q);
  float q4mag(quat q);
  quat  q4norm(quat q);
I quat  q4conj(quat q);
I quat  q4neg(quat q);
  quat  q4inv(quat q);
I quat  q4canon(quat q);
I float q4dot(quat a, quat b);
  quat  q4mul(quat a, quat b);
  quat  q4lerp(quat a, quat b, float t);
  quat  q4nlerp(quat a, quat b, float t);
  quat  q4slerp(quat a, quat b, float t);
  quat  q4axang(vec3 axis, float angle);
  quat  q4euler(vec3 euler);
  quat  q4m(mat3 matrix);
  float q4angle(quat q);
  vec3  q4axis(quat q);

  vec3  v3rotate(vec3 v, quat q);
  quat  v3rotation(vec3 from, vec3 to);
  quat  v3look(vec3 forward, vec3 up);

I vec3 q4dir(quat q);
I vec3 q4right(quat q);
I vec3 q4up(quat q);

////////////////////////////////////////////////////////////////////////////////
// Conversion and construction
////////////////////////////////////////////////////////////////////////////////

I quat q4f(float i, float j, float k, float w) {
  return (quat) { .i = i, .j = j, .k = k, .w = w };
}

I quat qv34f(vec3 ijk, float w) {
  return (quat) { .i = ijk.x, .j = ijk.y, .k = ijk.z, .w = w };
}

////////////////////////////////////////////////////////////////////////////////

I float q4magsq(quat q) {
  return q.i * q.i + q.j * q.j + q.k * q.k + q.w * q.w;
}

I quat q4conj(quat q) {
  return q4f(-q.i, -q.j, -q.k, q.w);
}

I quat q4neg(quat q) {
  return q4f(-q.i, -q.j, -q.k, -q.w);
}

I quat q4canon(quat q) {
  return q.w < 0.f ? q4neg(q) : q;
}

I float q4dot(quat a, quat b) {
  return a.i * b.i + a.j * b.j + a.k * b.k + a.w * b.w;
}

I vec3 q4dir(quat q) {
  return v3rotate(v3front, q);
}

I vec3 q4right(quat q) {
  return v3rotate(v3right, q);
}

I vec3 q4up(quat q) {
  return v3rotate(v3up, q);
}

#undef I

#endif

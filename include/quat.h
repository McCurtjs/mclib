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

#define q4front     q4identity
#define q4right     ((quat){.f={ 0, SQRT1_2, 0, SQRT1_2 } })
#define q4left      ((quat){.f={ 0,-SQRT1_2, 0, SQRT1_2 } })
#define q4up        ((quat){.f={-SQRT1_2, 0, 0, SQRT1_2 } })
#define q4down      ((quat){.f={ SQRT1_2, 0, 0, SQRT1_2 } })
#define q4back      ((quat){.f={ 0, 1, 0, 0 } })

#define I static inline

I float q4magsq(quat q);
  float q4mag(quat q);
  quat  q4norm(quat q);
I quat  q4norm_eq(quat* q);
I quat  q4conj(quat q);
I quat  q4neg(quat q);
  quat  q4inv(quat q);
I quat  q4canon(quat q);
I float q4dot(quat a, quat b);
  quat  q4mul(quat a, quat b);
I void  q4mul_eq(quat* a, quat b);
I void  q4premul_eq(quat a, quat* b);
  quat  q4pow(quat q, float t);
I quat  q4between(quat a, quat b);
  quat  q4lerp(quat a, quat b, float t);
  quat  q4nlerp(quat a, quat b, float t);
  quat  q4slerp(quat a, quat b, float t);
I quat  q4mix(quat a, quat b, float t);
  quat  q4axang(vec3 axis, float angle);
  quat  q4euler(vec3 euler);
I quat  q4from_to(vec3 from, vec3 to);
I quat  q4look(vec3 forward, vec3 up);
  quat  q4m(mat3 matrix);
  vec3  q4axis(quat q);
  float q4angle(quat q);
  float q4dist(quat a, quat b);
  bool  q4within(quat a, quat b, float angle);
I vec3  q4dir(quat q);

  vec3  v3euler(quat q);
  vec3  v3rotate(vec3 v, quat q);
I void  v3rotate_eq(vec3* v, quat q);
I vec3  v3rotate_slerp(vec3 v, quat q, float t);
I void  v3rotate_slerp_eq(vec3* v, quat q, float t);
  quat  v3rotation(vec3 from, vec3 to);
  quat  v3look(vec3 forward, vec3 up);

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

I quat q4between(quat a, quat b) {
  return q4mul(q4inv(a), b);
}

I void q4mul_eq(quat* a, quat b) {
  *a = q4mul(*a, b);
}

I void q4premul_eq(quat a, quat* b) {
  *b = q4mul(a, *b);
}

I quat q4mix(quat a, quat b, float t) {
  return q4nlerp(a, b, t);
}

I quat q4from_to(vec3 from, vec3 to) {
  return v3rotation(from, to);
}

I quat q4look(vec3 forward, vec3 up) {
  return v3look(forward, up);
}

I vec3 q4dir(quat q) {
  return v3rotate(v3front, q);
}

////////////////////////////////////////////////////////////////////////////////

I void v3rotate_eq(vec3* v, quat q) {
  *v = v3rotate(*v, q);
}

I vec3 v3rotate_slerp(vec3 v, quat q, float t) {
  return v3rotate(v, q4pow(q, t));
}

I void v3rotate_slerp_eq(vec3* v, quat q, float t) {
  *v = v3rotate(*v, q4pow(q, t));
}

#undef I

#endif

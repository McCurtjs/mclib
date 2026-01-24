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

#define q4zero       ((quat){.f={ 0, 0, 0, 0 } })
#define q4identity   ((quat){.f={ 0, 0, 0, 1 } })

float q4magsq(quat q);
float q4mag(quat q);
quat  q4norm(quat q);
quat  q4conj(quat q);
quat  q4inv(quat q);
quat  q4canon(quat q);
float q4dot(quat a, quat b);
quat  q4mul(quat a, quat b);
quat  q4lerp(quat a, quat b, float t);
quat  q4nlerp(quat a, quat b, float t);
quat  q4slerp(quat a, quat b, float t);
quat  q4axis(vec3 axis, float angle);
quat  q4euler(vec3 euler);
quat  q3rotation(vec3 from, vec3 to);
quat  q3look(vec3 forward, vec3 up);
quat  q4m(mat3 matrix);

vec3  v3rotate(vec3 v, quat q);

static inline quat q4f(float i, float j, float k, float w) {
  return (quat) { .i = i, .j = j, .k = k, .w = w };
}

static inline quat qv34f(vec3 ijk, float w) {
  return (quat) { ijk.x, ijk.y, ijk.z, w };
}

#endif

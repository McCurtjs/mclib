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

#ifndef _MCLIB_MATRIX_H_
#define _MCLIB_MATRIX_H_

#include "vec.h"

typedef struct mat2 {
  union {
    float f[4];
    float m[2][2];
    vec2 col[2];
  };
} mat2;

typedef struct mat3 {
  union {
    float f[9];
    float m[3][3];
    vec3  col[3];
  };
} mat3;

typedef struct mat4 {
  union {
    float f[16];
    float m[4][4];
    vec4  col[4];
  };
} mat4;

#define m2floats 4
#define m3floats 9
#define m4floats 16

#define m2identity ((mat2) {.f={  \
  1, 0,                           \
  0, 1,                           \
}})                              //

#define m2zero ((mat2) {.f={  \
  0, 0,                       \
  0, 0,                       \
}})                          //

#define m3identity ((mat3) {.f={  \
  1, 0, 0,                        \
  0, 1, 0,                        \
  0, 0, 1,                        \
}})                              //

#define m3zero ((mat3) {.f={  \
  0, 0, 0,                    \
  0, 0, 0,                    \
  0, 0, 0,                    \
}})                          //

#define m4identity ((mat4) {.f={  \
  1, 0, 0, 0,                     \
  0, 1, 0, 0,                     \
  0, 0, 1, 0,                     \
  0, 0, 0, 1                      \
}})                              //

#define m4zero ((mat4) {.f={  \
  0, 0, 0, 0,                 \
  0, 0, 0, 0,                 \
  0, 0, 0, 0,                 \
  0, 0, 0, 0                  \
}})                          //

mat3 m3mul(mat3 a, mat3 b);
vec3 mv3mul(mat3 m, vec3 v);
mat3 m3transpose(mat3 m);

mat4 m4ortho(
  float left, float right, float top, float bottom, float near, float far);
mat4 m4perspective(float fov_rads, float aspect, float near, float far);
mat4 m4basis(vec3 x, vec3 y, vec3 z, vec3 origin);
mat4 m4look(vec3 pos, vec3 target, vec3 up);
mat4 m4translation(vec3 vec);
mat4 m4rotation(vec3 axis, float angle);
mat4 m4scalar(float scalar);
mat4 m4vscalar(vec3 scalar);
mat4 m4mul(mat4 a, mat4 b);
vec4 mv4mul(mat4 m, vec4 v);
mat4 m4transpose(mat4 mat);
mat4 m4inverse(mat4 mat);

////////////////////////////////////////////////////////////////////////////////
// Matrix type conversion shorthands
////////////////////////////////////////////////////////////////////////////////

static inline mat2 m2f(float m00, float m01, float m10, float m11) {
  return (mat2) { .f = { m00, m01, m10, m11 } };
}

static inline mat2 m2v(vec2 A, vec2 B) {
  return (mat2) { .col = { A, B } };
}

static inline mat3 m23(mat2 m) {
  mat3 ret = m3identity;
  ret.col[0].xy = m.col[0];
  ret.col[1].xy = m.col[1];
  return ret;
}

static inline mat3 m3v(vec3 A, vec3 B, vec3 C) {
  mat3 ret;
  ret.col[0] = A;
  ret.col[1] = B;
  ret.col[2] = C;
  return ret;
}

static inline mat4 m4v(vec4 A, vec4 B, vec4 C, vec4 D) {
  mat4 ret;
  ret.col[0] = A;
  ret.col[1] = B;
  ret.col[2] = C;
  ret.col[3] = D;
  return ret;
}

static inline mat4 m4v3(vec3 A, vec3 B, vec3 C, vec3 D) {
  mat4 ret;
  ret.col[0] = v34f(A, 0);
  ret.col[1] = v34f(B, 0);
  ret.col[2] = v34f(C, 0);
  ret.col[3] = v34f(D, 1);
  return ret;
}

static inline mat4 m34(mat3 m) {
  mat4 ret;
  ret.col[0] = v34f(m.col[0], 0);
  ret.col[1] = v34f(m.col[1], 0);
  ret.col[2] = v34f(m.col[2], 0);
  ret.col[3] = v34f(m.col[2], 1);
  return ret;
}

static inline mat4 m34f(mat3 m, float tx, float ty, float tz) {
  mat4 ret = m34(m);
  ret.col[3].xyz = v3f(tx, ty, tz);
  return ret;
}

static inline mat4 m34v(mat3 m, vec3 translation) {
  mat4 ret = m34(m);
  ret.col[3].xyz = translation;
  return ret;
}

static inline mat3 m43(mat4 m) {
  mat3 ret;
  ret.col[0] = m.col[0].xyz;
  ret.col[1] = m.col[1].xyz;
  ret.col[2] = m.col[2].xyz;
  return ret;
}

#endif

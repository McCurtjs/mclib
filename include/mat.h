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

typedef struct {
  union {
    float f[16];
    float m[4][4];
    vec4  col[4];
  };
} mat4;

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

mat4 m4ortho(
  float left, float right, float top, float bottom, float near, float far);
mat4 m4perspective(float fov_rads, float aspect, float near, float far);
mat4 m4basis(vec3 x, vec3 y, vec3 z, vec3 origin);
mat4 m4look(vec3 pos, vec3 target, vec3 up);
mat4 m4translation(vec3 vec);
mat4 m4rotation(vec3 axis, float angle);
mat4 m4scalar(vec3 scalar);
mat4 m4uniform(float scalar);
mat4 m4mul(mat4 a, mat4 b);
vec4 mv4mul(mat4 m, vec4 v);
mat4 m4inverse(mat4 mat);

#endif

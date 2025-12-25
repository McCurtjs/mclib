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

#include "mat.h"

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// Matrix 3x3
////////////////////////////////////////////////////////////////////////////////

mat3 m3mul(mat3 a, mat3 b) {
  mat3 ret = m3zero;

  for (int x = 0; x < 3; ++x) {
    for (int y = 0; y < 3; ++y) {
      for (int i = 0; i < 3; ++i) {
        ret.m[x][y] += a.m[i][y] * b.m[x][i];
      }
    }
  }

  return ret;
}

vec3 mv3mul(mat3 m, vec3 v) {
  vec3 ret = v3zero;

  for (int y = 0; y < 3; ++y) {
    for (int x = 0; x < 3; ++x) {
      ret.f[y] += m.m[x][y] * v.f[x];
    }
  }

  return ret;
}

mat3 m3transpose(mat3 m) {
  mat3 ret;

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      ret.m[i][j] = m.m[j][i];
    }
  }

  return ret;
}

mat3 m3q(quat q) {
  float xx = q.x * q.x;
  float yy = q.y * q.y;
  float zz = q.z * q.z;
  float xy = q.x * q.y;
  float xz = q.x * q.z;
  float yz = q.y * q.z;
  float wx = q.w * q.x;
  float wy = q.w * q.y;
  float wz = q.w * q.z;

  return (mat3) { .f = {
    1.f - 2.f * (yy + zz) , 2.f * (xy + wz)       , 2.f * (xz - wy),
    2.f * (xy - wz)       , 1.f - 2.f * (xx + zz) , 2.f * (yz + wx),
    2.f * (xz + wy)       , 2.f * (yz - wx)       , 1.f - 2.f * (xx + yy)
  }};
}

////////////////////////////////////////////////////////////////////////////////
// Matrix 4x4
////////////////////////////////////////////////////////////////////////////////

mat4 m4orthographic(
  float left, float right, float top, float bottom, float near, float far
) {
  mat4 ret = m4identity;

  // this part scales to the NDC space
  ret.m[0][0] = 2.f / (right - left);
  ret.m[1][1] = 2.f / (top - bottom);
  ret.m[2][2] = -2.f / (far - near);

  ret.m[3][0] = -(right + left) / (right - left);
  ret.m[3][1] = -(top + bottom) / (top - bottom);
  ret.m[3][2] = -(far + near) / (far - near);

  return ret;
}

// https://jsantell.com/3d-projection/
mat4 m4perspective(float fov_rads, float aspect, float near, float far) {
  mat4 ret = m4zero;

  float S = 1.f / tanf(fov_rads / 2.f);
  float fmn = far - near;

  ret.m[0][0] = S / aspect;
  ret.m[1][1] = S;
  ret.m[2][2] = -far / fmn;
  ret.m[3][2] = (-far * near) / fmn;
  ret.m[2][3] = -1;

  return ret;
}

mat4 m4basis(vec3 x, vec3 y, vec3 z, vec3 origin) {
  return (mat4) {.f={
    x.x, y.x, z.x, 0,
    x.y, y.y, z.y, 0,
    x.z, y.z, z.z, 0,
    -v3dot(x, origin), -v3dot(y, origin), -v3dot(z, origin), 1
  }};
}

mat4 m4look(vec3 pos, vec3 target, vec3 up) {
  vec3 cz = v3norm(v3sub(target, pos)); // front
  vec3 cx = v3norm(v3cross(up, cz)); // left
  vec3 cy = v3cross(cz, cx); // up

  return m4inverse(m4basis(cx, cy, cz, pos));
}

mat4 m4translation(vec3 vec) {
  mat4 ret = m4identity;
  ret.col[3].xyz = vec;
  return ret;
}

mat4 m4rotation(vec3 axis, float angle) {
  mat4 ret;

  float s = sinf(angle);
  float c = cosf(angle);
  float cd1 = 1 - c;

  ret.m[0][0] = cd1 * axis.x * axis.x + c;
  ret.m[0][1] = cd1 * axis.y * axis.x + axis.z * s;
  ret.m[0][2] = cd1 * axis.z * axis.x - axis.y * s;
  ret.m[0][3] = 0;

  ret.m[1][0] = cd1 * axis.x * axis.y - axis.z * s;
  ret.m[1][1] = cd1 * axis.y * axis.y + c;
  ret.m[1][2] = cd1 * axis.z * axis.y + axis.x * s;
  ret.m[1][3] = 0;

  ret.m[2][0] = cd1 * axis.x * axis.z + axis.y * s;
  ret.m[2][1] = cd1 * axis.y * axis.z - axis.x * s;
  ret.m[2][2] = cd1 * axis.z * axis.z + c;
  ret.m[2][3] = 0;

  ret.col[3] = v4w;

  return ret;
}

mat4 m4q(quat q) {
  float xx = q.x * q.x;
  float yy = q.y * q.y;
  float zz = q.z * q.z;
  float xy = q.x * q.y;
  float xz = q.x * q.z;
  float yz = q.y * q.z;
  float wx = q.w * q.x;
  float wy = q.w * q.y;
  float wz = q.w * q.z;

  return (mat4) {.f = {
    1.f - 2.f * (yy + zz) , 2.f * (xy + wz)       , 2.f * (xz - wy)       , 0,
    2.f * (xy - wz)       , 1.f - 2.f * (xx + zz) , 2.f * (yz + wx)       , 0,
    2.f * (xz + wy)       , 2.f * (yz - wx)       , 1.f - 2.f * (xx + yy) , 0,
    0                     , 0                     , 0                     , 1
  }};
}

mat4 m4scalar(float scalar) {
  mat4 ret = m4identity;
  ret.m[0][0] = scalar;
  ret.m[1][1] = scalar;
  ret.m[2][2] = scalar;
  return ret;
}

mat4 m4vscalar(vec3 scalar) {
  mat4 ret = m4identity;
  ret.m[0][0] = scalar.x;
  ret.m[1][1] = scalar.y;
  ret.m[2][2] = scalar.z;
  return ret;
}

mat4 m4mul(mat4 a, mat4 b) {
  mat4 ret = m4zero;

  for (int x = 0; x < 4; ++x) {
    for (int y = 0; y < 4; ++y) {
      for (int i = 0; i < 4; ++i) {
        ret.m[x][y] += a.m[i][y] * b.m[x][i];
      }
    }
  }

  return ret;
}

vec4 mv4mul(mat4 m, vec4 v) {
  vec4 ret = v4zero;

  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      ret.f[y] += m.m[x][y] * v.f[x];
    }
  }

  return ret;
}

mat4 m4transpose(mat4 m) {
  mat4 ret;

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      ret.m[i][j] = m.m[j][i];
    }
  }

  return ret;
}

mat4 m4inverse(mat4 m) {
  return (mat4) {.f={
    m.f[5] * m.f[10] * m.f[15] - m.f[5] * m.f[11] * m.f[14] -
    m.f[9] * m.f[6] * m.f[15] + m.f[9] * m.f[7] * m.f[14] +
    m.f[13] * m.f[6] * m.f[11] - m.f[13] * m.f[7] * m.f[10]
    ,
    -m.f[1] * m.f[10] * m.f[15] + m.f[1] * m.f[11] * m.f[14] +
    m.f[9] * m.f[2] * m.f[15] - m.f[9] * m.f[3] * m.f[14] -
    m.f[13] * m.f[2] * m.f[11] + m.f[13] * m.f[3] * m.f[10]
    ,
    m.f[1] * m.f[6] * m.f[15] - m.f[1] * m.f[7] * m.f[14] -
    m.f[5] * m.f[2] * m.f[15] + m.f[5] * m.f[3] * m.f[14] +
    m.f[13] * m.f[2] * m.f[7] - m.f[13] * m.f[3] * m.f[6]
    ,
    -m.f[1] * m.f[6] * m.f[11] + m.f[1] * m.f[7] * m.f[10] +
    m.f[5] * m.f[2] * m.f[11] - m.f[5] * m.f[3] * m.f[10] -
    m.f[9] * m.f[2] * m.f[7] + m.f[9] * m.f[3] * m.f[6]
    ,
    -m.f[4] * m.f[10] * m.f[15] + m.f[4] * m.f[11] * m.f[14] +
    m.f[8] * m.f[6] * m.f[15] - m.f[8] * m.f[7] * m.f[14] -
    m.f[12] * m.f[6] * m.f[11] + m.f[12] * m.f[7] * m.f[10]
    ,
    m.f[0] * m.f[10] * m.f[15] - m.f[0] * m.f[11] * m.f[14] -
    m.f[8] * m.f[2] * m.f[15] + m.f[8] * m.f[3] * m.f[14] +
    m.f[12] * m.f[2] * m.f[11] - m.f[12] * m.f[3] * m.f[10]
    ,
    -m.f[0] * m.f[6] * m.f[15] + m.f[0] * m.f[7] * m.f[14] +
    m.f[4] * m.f[2] * m.f[15] - m.f[4] * m.f[3] * m.f[14] -
    m.f[12] * m.f[2] * m.f[7] + m.f[12] * m.f[3] * m.f[6]
    ,
    m.f[0] * m.f[6] * m.f[11] - m.f[0] * m.f[7] * m.f[10] -
    m.f[4] * m.f[2] * m.f[11] + m.f[4] * m.f[3] * m.f[10] +
    m.f[8] * m.f[2] * m.f[7] - m.f[8] * m.f[3] * m.f[6]
    ,
    m.f[4] * m.f[9] * m.f[15] - m.f[4] * m.f[11] * m.f[13] -
    m.f[8] * m.f[5] * m.f[15] + m.f[8] * m.f[7] * m.f[13] +
    m.f[12] * m.f[5] * m.f[11] - m.f[12] * m.f[7] * m.f[9]
    ,
    -m.f[0] * m.f[9] * m.f[15] + m.f[0] * m.f[11] * m.f[13] +
    m.f[8] * m.f[1] * m.f[15] - m.f[8] * m.f[3] * m.f[13] -
    m.f[12] * m.f[1] * m.f[11] + m.f[12] * m.f[3] * m.f[9]
    ,
    m.f[0] * m.f[5] * m.f[15] - m.f[0] * m.f[7] * m.f[13] -
    m.f[4] * m.f[1] * m.f[15] + m.f[4] * m.f[3] * m.f[13] +
    m.f[12] * m.f[1] * m.f[7] - m.f[12] * m.f[3] * m.f[5]
    ,
    -m.f[0] * m.f[5] * m.f[11] + m.f[0] * m.f[7] * m.f[9] +
    m.f[4] * m.f[1] * m.f[11] - m.f[4] * m.f[3] * m.f[9] -
    m.f[8] * m.f[1] * m.f[7] + m.f[8] * m.f[3] * m.f[5]
    ,
    -m.f[4] * m.f[9] * m.f[14] + m.f[4] * m.f[10] * m.f[13] +
    m.f[8] * m.f[5] * m.f[14] - m.f[8] * m.f[6] * m.f[13] -
    m.f[12] * m.f[5] * m.f[10] + m.f[12] * m.f[6] * m.f[9]
    ,
    m.f[0] * m.f[9] * m.f[14] - m.f[0] * m.f[10] * m.f[13] -
    m.f[8] * m.f[1] * m.f[14] + m.f[8] * m.f[2] * m.f[13] +
    m.f[12] * m.f[1] * m.f[10] - m.f[12] * m.f[2] * m.f[9]
    ,
    -m.f[0] * m.f[5] * m.f[14] + m.f[0] * m.f[6] * m.f[13] +
    m.f[4] * m.f[1] * m.f[14] - m.f[4] * m.f[2] * m.f[13] -
    m.f[12] * m.f[1] * m.f[6] + m.f[12] * m.f[2] * m.f[5]
    ,
    m.f[0] * m.f[5] * m.f[10] - m.f[0] * m.f[6] * m.f[9] -
    m.f[4] * m.f[1] * m.f[10] + m.f[4] * m.f[2] * m.f[9] +
    m.f[8] * m.f[1] * m.f[6] - m.f[8] * m.f[2] * m.f[5]
  }};
}

////////////////////////////////////////////////////////////////////////////////
// Transform shorthand
////////////////////////////////////////////////////////////////////////////////

mat4 m4trs(vec3 translation, vec3 axis, float angle, float uniform_scalar) {
  mat4 ret = m4rs(axis, angle, uniform_scalar);
  ret.col[3].xyz = translation;
  return ret;
}

mat4 m4trsv(vec3 translation, vec3 axis, float angle, vec3 non_uniform_scalar) {
  mat4 ret = m4rsv(axis, angle, non_uniform_scalar);
  ret.col[3].xyz = translation;
  return ret;
}

mat4 m4trsq(vec3 translation, quat q, float uniform_scalar) {
  mat4 ret = m4rsq(q, uniform_scalar);
  ret.col[3].xyz = translation;
  return ret;
}

mat4 m4trsqv(vec3 translation, quat q, vec3 non_uniform_scalar) {
  mat4 ret = m4rsqv(q, non_uniform_scalar);
  ret.col[3].xyz = translation;
  return ret;
}

mat4 m4ts(vec3 translation, float uniform_scalar) {
  return m4mul(m4translation(translation), m4scalar(uniform_scalar));
}

mat4 m4tsv(vec3 translation, vec3 non_uniform_scalar) {
  return m4mul(m4translation(translation), m4vscalar(non_uniform_scalar));
}

mat4 m4tr(vec3 translation, vec3 axis, float angle) {
  mat4 ret = m4rotation(axis, angle);
  ret.col[3].xyz = translation;
  return ret;
}

mat4 m4trq(vec3 translation, quat q) {
  mat4 ret = m4q(q);
  ret.col[3].xyz = translation;
  return ret;
}

mat4 m4rs(vec3 axis, float angle, float scale) {
  return m4rsv(axis, angle, v3f(scale, scale, scale));
}

mat4 m4rsv(vec3 axis, float angle, vec3 scale) {
  mat4 ret = m4rotation(axis, angle);
  ret.m[0][0] *= scale.x;
  ret.m[0][1] *= scale.x;
  ret.m[0][2] *= scale.x;
  ret.m[1][0] *= scale.y;
  ret.m[1][1] *= scale.y;
  ret.m[1][2] *= scale.y;
  ret.m[2][0] *= scale.z;
  ret.m[2][1] *= scale.z;
  ret.m[2][2] *= scale.z;
  return ret;
}

mat4 m4rsq(quat q, float scale) {
  return m4rsqv(q, v3f(scale, scale, scale));
}

mat4 m4rsqv(quat q, vec3 scale) {
  mat4 ret = m4q(q);
  ret.m[0][0] *= scale.x;
  ret.m[0][1] *= scale.x;
  ret.m[0][2] *= scale.x;
  ret.m[1][0] *= scale.y;
  ret.m[1][1] *= scale.y;
  ret.m[1][2] *= scale.y;
  ret.m[2][0] *= scale.z;
  ret.m[2][1] *= scale.z;
  ret.m[2][2] *= scale.z;
  return ret;
}

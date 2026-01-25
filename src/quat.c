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

#include "quat.h"

#include <math.h>

const float epsilon = 1e-6f;

float q4magsq(quat q) {
  return q.i*q.i + q.j*q.j + q.k*q.k + q.w*q.w;
}

float q4mag(quat q) {
  return sqrtf(q4magsq(q));
}

quat q4norm(quat q) {
  float magsq = q4magsq(q);
  if (magsq <= 0.0f) return q4identity;
  float inv = 1.0f / sqrtf(magsq);
  return q4f(q.i*inv, q.j*inv, q.k*inv, q.w*inv);
}

quat q4conj(quat q) {
  return q4f(-q.i, -q.j, -q.k, -q.w);
}

quat q4inv(quat q) {
  float magsq = q4magsq(q);
  if (magsq <= 0.0f) return q4identity;
  float inv = -1.0f / magsq;
  return q4f(q.i*inv, q.j*inv, q.k*inv, q.w*inv);
}

quat q4canon(quat q) {
  if (q.w >= 0.0f) return q;
  return q4conj(q);
}

float q4dot(quat a, quat b) {
  return a.i*b.i + a.j*b.j + a.k*b.k + a.w*b.w;
}

quat q4mul(quat a, quat b) {
  return (quat) {
    .i = a.w*b.i + a.i*b.w + a.j*b.k - a.k*b.j,
    .j = a.w*b.j - a.i*b.k + a.j*b.w + a.k*b.i,
    .k = a.w*b.k + a.i*b.j - a.j*b.i + a.k*b.w,
    .w = a.w*b.w - a.i*b.i - a.j*b.j - a.k*b.k,
  };
}

quat q4lerp(quat a, quat b, float t) {
  return (quat) {
    .i = a.i + (b.i - a.i) * t,
    .j = a.j + (b.j - a.j) * t,
    .k = a.k + (b.k - a.k) * t,
    .w = a.w + (b.w - a.w) * t
  };
}

quat q4nlerp(quat a, quat b, float t) {
  if (q4dot(a, b) < 0.0f) b = q4conj(b);
  return q4norm(q4lerp(a, b, t));
}

quat q4slerp(quat a, quat b, float t) {
  float cosom = q4dot(a, b);
  if (cosom < 0.0f) {
    cosom = -cosom;
    b = q4conj(b);
  }

  if (1.0 - cosom < epsilon) {
    return q4nlerp(a, b, t);
  }

  float omega = acosf(cosom);
  float sinom = sinf(omega);

  float s0 = sinf((1.0f - t) * omega) / sinom;
  float s1 = sinf(t * omega);

  quat ret = (quat) {
    .i = a.i*s0 + b.i*s1,
    .j = a.j*s0 + b.j*s1,
    .k = a.k*s0 + b.k*s1,
    .w = a.w*s0 + b.w*s1,
  };
  return q4norm(ret);
}

quat q4axang(vec3 axis, float angle) {
  axis = v3norm(axis);
  float half = 0.5f * angle;
  float s = sinf(half);
  float c = cosf(half);
  return q4norm(q4f(axis.x * s, axis.y * s, axis.z * s, c));
}

quat q4euler(vec3 euler) {
  // yaw, pitch, roll
  quat qy = q4axang(v3up, euler.y);
  quat qx = q4axang(v3right, euler.x);
  quat qz = q4axang(v3front, euler.z);
  return q4norm(q4mul(q4mul(qy, qx), qz));
}

quat q3rotation(vec3 from, vec3 to) {
  from = v3norm(from);
  to = v3norm(to);

  float d = v3dot(from, to);
  if (d > 1.0f - epsilon) return q4identity;

  // if vectors are nearly opposite, pick some orthogonal axis
  if (d < -1.0f + epsilon) {
    return q4axang(v3perp(from), PI);
  }

  vec3 axis = v3cross(from, to);
  return q4norm(qv34f(axis, 1.0f + d));
}

quat q3look(vec3 forward, vec3 up) {
  vec3 f = v3norm(forward);

  if (v3magsq(f) < epsilon) return q4identity;

  vec3 r = v3cross(f, up);
  float rmagsq = v3magsq(r);
  if (rmagsq < epsilon) {
    up = (fabsf(f.y) < 1.0f - epsilon) ? v3up : v3right;
    r = v3cross(f, up);
  }
  r = v3norm(r);

  vec3 u = v3cross(r, f);

  mat3 basis = m3v(r, u, v3neg(f));
  assert(m3det(basis) > 0.0f); // right-handed check
  return q4m(basis);
}

quat q4m(mat3 m) {
  float trace = m3trace(m);
  quat q;

  if (trace > 0.0f) {
    float s = sqrtf(trace + 1.0f) * 2.0f; // s = 4*w
    q = q4f(
      (m.col[2].y - m.col[1].z) / s,
      (m.col[0].z - m.col[2].x) / s,
      (m.col[1].x - m.col[0].y) / s,
      0.25f * s
    );
  }
  else if (m.col[0].x > m.col[1].y && m.col[0].x > m.col[2].z) {
    // s = 4*x
    float s = sqrtf(1.0f + m.col[0].x - m.col[1].y - m.col[2].z) * 2.0f;
    q = q4f(
      0.25f * s,
      (m.col[0].y + m.col[1].x) / s,
      (m.col[0].z + m.col[2].x) / s,
      (m.col[2].y - m.col[1].z) / s
    );
  }
  else if (m.col[1].y > m.col[2].z) {
    // s = 4*y
    float s = sqrtf(1.0f + m.col[1].y - m.col[0].x - m.col[2].z) * 2.0f;
    q = q4f(
      (m.col[0].y + m.col[1].x) / s,
      0.25f * s,
      (m.col[1].z + m.col[2].y) / s,
      (m.col[0].z - m.col[2].x) / s
    );
  }
  else {
    // s = 4*z
    float s = sqrtf(1.0f + m.col[2].z - m.col[0].x - m.col[1].y) * 2.0f;
    q = q4f(
      (m.col[0].z + m.col[2].x) / s,
      (m.col[1].z + m.col[2].y) / s,
      0.25f * s,
      (m.col[1].x - m.col[0].y) / s
    );
  }

  return q4canon(q4norm(q));
}

float q4angle(quat q) {
  return 2.0f * acosf(q.w);
}

vec3 q4axis(quat q) {
  q = q4norm(q);
  float s = sqrtf(1.0f - q.w*q.w);
  if (s < epsilon)
    return v3right;
  return v3scale(q.ijk, s);
}

////////////////////////////////////////////////////////////////////////////////

vec3 v3rotate(vec3 v, quat q) {
  float v_mag = v3mag(v);

  // t = 2 * (qv x v)
  vec3 t = v3cross(q.ijk, v);

  // v' = v + q.w*t + (qv x t)
  vec3 qv_x_t = v3cross(q.ijk, t);
  return v3f(
    v.x + q.w*t.x + qv_x_t.x,
    v.y + q.w*t.y + qv_x_t.y,
    v.z + q.w*t.z + qv_x_t.z
  );
}

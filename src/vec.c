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

#include "vec.h"

#include <math.h>

// Based on community post from:
// https://community.khronos.org/t/quaternion-functions-for-glsl/50140/2
vec3 qtransform(quat q, vec3 v) {
  return v3add(
    v, v3scale(
      v3cross(
        v3add(v3cross(v, q.ijk), v3scale(v, q.w)),
        q.ijk
      )
    , 2)
  );
  //return v + 2.0*cross(cross(v, q.xyz ) + q.w*v, q.xyz);
}

////////////////////////////////////////////////////////////////////////////////
// Vector 2 (integer)
////////////////////////////////////////////////////////////////////////////////

float i2aspect(vec2i v) {
  return (float)v.w / (float)v.h;
}

vec2i i2zcurve(size_t i) {
  int x = 0, y = 0;
  size_t mask = 0x1;
  while (i) {
    x |= mask & i;
    i >>= 1;
    y |= mask & i;
    mask <<= 1;
  }
  return v2i(x, y);
}

size_t i2zindex(vec2i v) {
  size_t ret = 0, mask = 0x1;
  size_t x = (size_t)v.x, y = (size_t)v.y;
  while (mask) {
    ret |= mask & x;
    mask <<= 1;
    y <<= 1;
    ret |= mask & y;
    mask <<= 1;
    x <<= 1;
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
// Vector 2 (float)
////////////////////////////////////////////////////////////////////////////////

float v2mag(vec2 v) {
  return sqrtf(v2magsq(v));
}

float v2magsq(vec2 v) {
  return v.x * v.x + v.y * v.y;
}

float v2dist(vec2 P, vec2 Q) {
  return sqrtf(v2distsq(P, Q));
}

float v2distsq(vec2 P, vec2 Q) {
  return v2magsq(v2sub(Q, P));
}

vec2 v2norm(vec2 v) {
  float mag = v2mag(v);
  return v2f(v.x / mag, v.y / mag);
}

vec2 v2neg(vec2 v) {
  return v2f(-v.x, -v.y);
}

vec2 v2add(vec2 a, vec2 b) {
  return v2f(a.x + b.x, a.y + b.y);
}

vec2 v2sub(vec2 a, vec2 b) {
  return v2f(a.x - b.x, a.y - b.y);
}

vec2 v2scale(vec2 v, float f) {
  return v2f(v.x * f, v.y * f);
}

float v2dot(vec2 a, vec2 b) {
  return a.x * b.x + a.y * b.y;
}

vec2 v2mul(vec2 a, vec2 b) {
  return v2f(a.x * b.x, a.y * b.y);
}

float v2cross(vec2 a, vec2 b) {
  return a.x * b.y - a.y * b.x;
}

vec2 v2perp(vec2 v) {
  return v2f(-v.y, v.x);
}

vec2 v2reflect(vec2 v, vec2 mirror) {
  float t = v2dot(v, v2norm(mirror)) * v2mag(mirror);
  vec2 P = v2scale(mirror, t);
  vec2 r = v2sub(P, v);
  return v2add(P, r);
}

float v2angle(vec2 a, vec2 b) {
  return acosf(v2dot(a, b) / (v2mag(a) * v2mag(b)));
}

vec2 v2dir(float theta) {
  float sint, cost;
  //sincosf(theta, &sint, &cost);
  sint = sinf(theta); cost = sinf(theta);
  return v2f( cost, sint );
}

vec2 v2rot(vec2 v, float theta) {
  float sint, cost;
  //sincosf(theta, &sint, &cost);
  sint = sinf(theta); cost = cosf(theta);
  return v2f( cost * v.x - sint * v.y, sint * v.x + cost * v.y );
}

vec2 v2lerp(vec2 P, vec2 Q, float t) {
  vec2 v = v2scale(v2sub(Q, P), t);
  return v2add(P, v);
}

float v2line_dist(vec2 L, vec2 v, vec2 P) {
  return v2cross(v2sub(P, L), v) / v2mag(v);
}

// todo: change return types, look up other version of formula that uses t?
//       (should return t value like intersections?)
float v2line_closest(vec2 L, vec2 v, vec2 P, vec2* R_out) {
  float d = v2line_dist(L, v, P);
  if (!R_out) return d;
  vec2 n = v2norm(v2perp(v));
  *R_out = v2add(P, v2scale(n, d));
  return d;
}

// t = (L.x * u.y - L.y * u.x + u.x * Q.y - u.y * Q.x) / (u.x * v.y - u.y * v.x)
// s = (v.x * L.y - v.y * L.x + Q.x * v.y - Q.y * v.x) / (v.x * u.y - v.y * u.x)
bool v2line_line(vec2 L, vec2 v, vec2 Q, vec2 u, float* t_out, float* s_out) {
  float div = v2cross(u, v);
  if (div == 0) return FALSE;
  if (t_out) *t_out = (v2cross(L, u) + v2cross(u, Q)) / div;
  if (s_out) *s_out = (v2cross(v, L) + v2cross(Q, v)) / -div;
  return TRUE;
}

bool v2ray_line(vec2 R, vec2 v, vec2 L, vec2 u, float* t_out) {
  float t;
  if (!v2line_line(R, v, L, u, &t, NULL)) return FALSE;
  if (t_out) *t_out = t;
  if (t < 0) return FALSE;
  return TRUE;
}

bool v2ray_ray(vec2 R, vec2 v, vec2 Q, vec2 u, float* t_out, float* s_out) {
  float t, s;
  if (!v2line_line(R, v, Q, u, &t, &s)) return FALSE;
  if (t_out) *t_out = t;
  if (s_out) *s_out = s;
  if (t < 0) return FALSE;
  if (s < 0) return FALSE;
  return TRUE;
}

bool v2ray_seg(vec2 L, vec2 v, vec2 S1, vec2 S2, float* t_out) {
  vec2 u = v2sub(S2, S1);
  float t, s; float* t_ptr = t_out ? t_out : &t;
  if (!v2ray_ray(L, v, S1, u, t_ptr, &s)) return FALSE;
  if (s > 1) return FALSE;
  return TRUE;
}

bool v2seg_seg(vec2 S1, vec2 S2, vec2 Q1, vec2 Q2, vec2* out) {
  vec2 v = v2sub(S2, S1), u = v2sub(Q2, Q1);
  float t, s;
  if (!v2ray_ray(S1, v, Q1, u, &t, &s)) return FALSE;
  if (t > 1) return FALSE;
  if (s > 1) return FALSE;
  if (out) *out = v2add(S1, v2scale(v, t));
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Vector 3 (float)
////////////////////////////////////////////////////////////////////////////////

float v3mag(vec3 v) {
  return sqrtf(v3magsq(v));
}

float v3magsq(vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

vec3 v3norm(vec3 v) {
  float mag = v3mag(v);
  return v3f( v.x / mag, v.y / mag, v.z / mag );
}

vec3 v3neg(vec3 v) {
  return v3f( -v.x, -v.y, -v.z );
}

vec3 v3add(vec3 a, vec3 b) {
  return v3f( a.x + b.x, a.y + b.y, a.z + b.z );
}

vec3 v3sub(vec3 a, vec3 b) {
  return v3f( a.x - b.x, a.y - b.y, a.z - b.z );
}

vec3 v3scale(vec3 a, float f) {
  return v3f( a.x * f, a.y * f, a.z * f );
}

float v3dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3  v3mul(vec3 a, vec3 b) {
  return v3f( a.x * b.x, a.y * b.y, a.z * b.z );
}

vec3 v3cross(vec3 a, vec3 b) {
  return v3f(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  );
}

// Gets an arbitrary vector that's perpendicular to v
//
// From Ken Whatmough's post on
// https://math.stackexchange.com/questions/137362/how-to-find-perpendicular-vector-to-another-vector
vec3 v3perp(vec3 v) {
  return v3f(
    copysignf(v.z, v.x),
    copysignf(v.z, v.y),
    -copysignf((float)fabs(v.x) + (float)fabs(v.y), v.z)
    // or -copysignf(v.x, v.z) - copysignf(v.y, v.z)
  );
}

float v3angle(vec3 a, vec3 b) {
  return acosf(v3dot(a, b) / (v3mag(a) * v3mag(b)));
}

vec3 v3lerp(vec3 P, vec3 Q, float t) {
  vec3 v = v3scale(v3sub(Q, P), t);
  return v3add(P, v);
}

float v3line_dist(vec3 L, vec3 v, vec3 P) {
  return v3mag(v3cross(v3sub(P, L), v)) / v3mag(v);
}

// Gets the intersection between the line [L, v] and plane [P, n]
//
// @param t_out If non-null and return value is TRUE, is set to t value of the
//              intersection point described by P + v * t_out
// @returns TRUE on intersection, FALSE if objects are parallel
bool v3line_plane(vec3 L, vec3 v, vec3 P, vec3 n, float* t_out) {
  vec3 norm = v3norm(n);
  float vdotn = v3dot(v, norm);
  if (vdotn == 0) return false;
  vec3 LtoP = v3sub(P, L);
  float LPdotn = v3dot(LtoP, norm);
  float t = LPdotn / vdotn;
  if (t_out) *t_out = t;
  return true;
}

// Gets the intersection between the ray [R, v] and plane [P, n]
//
// @param t_out If non-null and return value is TRUE, is set to t value of the
//              intersection point described by P + v * t_out
// @returns TRUE on intersection, FALSE if no intersection
bool v3ray_plane(vec3 R, vec3 v, vec3 P, vec3 n, float* t_out) {
  float t;
  if (!v3line_plane(R, v, P, n, &t)) return false;
  if (t < 0) return false;
  if (t_out) *t_out = t;
  return true;
}

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

vec3b v3b(byte r, byte g, byte b) {
  return (vec3b){.i={r, g, b}};
}

vec4b v4b(byte r, byte g, byte b, byte a) {
  return (vec4b){.i={r, g, b, a}};
}

float i2aspect(vec2i v) {
  return (float)v.w / (float)v.h;
}

vec2i i2zcurve(size_t i) {
  int x = 0, y = 0;
  size_t mask = 0x1, k = 0;
  while (i) {
    x |= mask & i;
    i >>= 1;
    y |= mask & i;
    mask <<= 1;
  }
  return v2i(x, y);
}

size_t i2zindex(vec2i v) {
  size_t ret = 0, k = 0, mask = 0x1;
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

vec2i v2i(int x, int y) {
  return (vec2i){.i={x, y}};
}

vec3i v3i(int x, int y, int z) {
  return (vec3i){.i={x, y, z}};
}

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

vec2 v2had(vec2 a, vec2 b) {
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

float v2line_dist(vec2 P, vec2 v, vec2 Q) {
  return v2cross(v2sub(Q, P), v) / v2mag(v);
}

// todo: change return types, look up other version of formula that uses t?
//       (should return t value like intersections?)
float v2line_closest(vec2 P, vec2 v, vec2 Q, vec2* R_out) {
  float d = v2line_dist(P, v, Q);
  if (!R_out) return d;
  vec2 n = v2norm(v2perp(v));
  *R_out = v2add(Q, v2scale(n, d));
  return d;
}

// t = (P.x * u.y - P.y * u.x + u.x * Q.y - u.y * Q.x) / (u.x * v.y - u.y * v.x)
// s = (v.x * P.y - v.y * P.x + Q.x * v.y - Q.y * v.x) / (v.x * u.y - v.y * u.x)
bool v2line_line(vec2 P, vec2 v, vec2 Q, vec2 u, float* t_out, float* s_out) {
  float div = v2cross(u, v);
  if (div == 0) return FALSE;
  if (t_out) *t_out = (v2cross(P, u) + v2cross(u, Q)) / div;
  if (s_out) *s_out = (v2cross(v, P) + v2cross(Q, v)) / -div;
  return TRUE;
}

bool v2ray_line(vec2 P, vec2 v, vec2 Q, vec2 u, float* t_out) {
  float t;
  if (!v2line_line(P, v, Q, u, &t, NULL)) return FALSE;
  if (t_out) *t_out = t;
  if (t < 0) return FALSE;
  return TRUE;
}

bool v2ray_ray(vec2 P, vec2 v, vec2 Q, vec2 u, float* t_out, float* s_out) {
  float t, s;
  if (!v2line_line(P, v, Q, u, &t, &s)) return FALSE;
  if (t_out) *t_out = t;
  if (s_out) *s_out = s;
  if (t < 0) return FALSE;
  if (s < 0) return FALSE;
  return TRUE;
}

bool v2ray_seg(vec2 P, vec2 v, vec2 Q1, vec2 Q2, float* t_out) {
  vec2 u = v2sub(Q2, Q1);
  float t, s; float* t_ptr = t_out ? t_out : &t;
  if (!v2ray_ray(P, v, Q1, u, t_ptr, &s)) return FALSE;
  if (s > 1) return FALSE;
  return TRUE;
}

bool v2seg_seg(vec2 P1, vec2 P2, vec2 Q1, vec2 Q2, vec2* out) {
  vec2 v = v2sub(P2, P1), u = v2sub(Q2, Q1);
  float t, s;
  if (!v2ray_ray(P1, v, Q1, u, &t, &s)) return FALSE;
  if (t > 1) return FALSE;
  if (s > 1) return FALSE;
  if (out) *out = v2add(P1, v2scale(v, t));
  return TRUE;
}

vec2 v2f(float x, float y) {
  return (vec2){.f = {x, y} };
}

vec3 v23(vec2 v) {
  return v3f(v.x, v.y, 0);
}

vec4 v24(vec2 v) {
  return v4f(v.x, v.y, 0, 0);
}

vec4 p24(vec2 v) {
  return v4f(v.x, v.y, 0, 1);
}

vec3 v23f(vec2 v, float z) {
  return v3f(v.x, v.y, z);
}

vec4 v24f(vec2 v, float z, float w) {
  return v4f(v.x, v.y, z, w);
}

vec4 p24f(vec2 v, float z) {
  return v4f(v.x, v.y, z, 1);
}

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

vec3  v3had(vec3 a, vec3 b) {
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

float v3line_dist(vec3 P, vec3 v, vec3 Q) {
  return v3mag(v3cross(v3sub(Q, P), v)) / v3mag(v);
}

// Gets the intersection between the line [P, v] and plane [R, n]
//
// @param t_out If non-null and return value is TRUE, is set to t value of the
//              intersection point described by P + v * t_out
// @returns TRUE on intersection, FALSE if objects are parallel
bool v3line_plane(vec3 P, vec3 v, vec3 R, vec3 n, float* t_out) {
  vec3 norm = v3norm(n);
  float vdotn = v3dot(v, norm);
  if (vdotn == 0) return FALSE;
  vec3 PtoR = v3sub(R, P);
  float PRdotn = v3dot(PtoR, norm);
  // if (lpdotn == 0) return TRALUSE? // line contained in plane
  float t = PRdotn / vdotn;
  if (t_out) *t_out = t;
  return t != 0;
}

// Gets the intersection between the ray [P, v] and plane [R, n]
//
// @param t_out If non-null and return value is TRUE, is set to t value of the
//              intersection point described by P + v * t_out
// @returns TRUE on intersection, FALSE if no intersection
bool v3ray_plane(vec3 P, vec3 v, vec3 R, vec3 n, float* t_out) {
  float t;
  if (!v3line_plane(P, v, R, n, &t)) return FALSE;
  if (t < 0) return FALSE;
  if (t_out) *t_out = t;
  return TRUE;
}

// Constructs a vec3 out of floats.
// Generally would prefer to use the form "(vec3){x, y, z}", but gcc is annoying
// about that syntax, throwing -Wmissing-braces because it actaully wants the
// form "(vec3){ .f = {x, y, z}}", which is annoying to type, so here we are.
vec3 v3f(float x, float y, float z) {
  return (vec3){.f = {x, y, z} };
}

// Converts a vec3 to a vec4 with homogeneous w component set to 0
vec4 v34(vec3 v) {
  return v4f(v.x, v.y, v.z, 0);
}

// Converts a vec3 to a vec4 with homogeneous w component set to 1
vec4 p34(vec3 p) {
  return v4f(p.x, p.y, p.z, 1);
}

// Converts a vec3 and a given float w to a vec4
vec4 v34f(vec3 v, float w) {
  return v4f(v.x, v.y, v.z, w);
}

vec4 v4f(float x, float y, float z, float w) {
  return (vec4){ .f = {x, y, z, w} };
}

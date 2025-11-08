/*******************************************************************************
* MIT License
*
* Copyright (c) 2025 Curtis McCoy
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

#ifdef MCLIB_STRING_H_

////////////////////////////////////////////////////////////////////////////////
// Default set of supported arg types available with str.h
// - string types: String, slice_t
// - C-style strings: char*, const char*
// - String collections: Array_slice, span_slice_t
// - basic types: int, float, long, double, etc.
////////////////////////////////////////////////////////////////////////////////

#ifndef MCLIB_STR_FORMAT_STRING_
#define MCLIB_STR_FORMAT_STRING_

static inline _str_arg_t _sarg_str(const String s) {
  return (_str_arg_t) { .type = _str_arg_slice, .slice = s->slice };
}

static inline _str_arg_t _sarg_slice(slice_t r) {
  return (_str_arg_t) { .type = _str_arg_slice, .slice = r };
}

static inline _str_arg_t _sarg_slice_ptr(const slice_t* r) {
  if (!r) r = &slice_empty;
  return (_str_arg_t) { .type = _str_arg_slice, .slice = *r };
}

static inline _str_arg_t _sarg_span(span_slice_t s) {
  return (_str_arg_t) { .type = _str_arg_span, .span = s };
}

static inline _str_arg_t _sarg_array(Array_slice a) {
  return (_str_arg_t) { .type = _str_arg_span, .span = a->span };
}

static inline _str_arg_t _sarg_c_str(const char* c_str) {
  if (!c_str) c_str = slice_empty.begin;
  return (_str_arg_t) {
    .type = _str_arg_slice, .slice = slice_from_c_str(c_str)
  };
}

static inline _str_arg_t _sarg_int(long long int i) {
  return (_str_arg_t) { .type = _str_arg_int, .i = i };
}

static inline _str_arg_t _sarg_unsigned(long long unsigned int i) {
  return (_str_arg_t) { .type = _str_arg_int, .i = (long long)i };
}

static inline _str_arg_t _sarg_float(double f) {
  return (_str_arg_t) { .type = _str_arg_float, .f = f };
}

#endif // end str_format basics

////////////////////////////////////////////////////////////////////////////////
// Adds support for format args of types from vec.h
// - float vector types: vec2, vec3, vec4
// - integer vector types: vec2i, vec3i
////////////////////////////////////////////////////////////////////////////////

#ifndef MCLIB_VECTOR_H_
# define MCLIB_STR_FORMAT_VECTOR_
#else

# ifdef MCLIB_STR_FORMAT_VECTOR_
#   undef MCLIB_STR_FORMAT_VECTOR_
# endif

# ifndef MCLIB_STR_FORMAT_VECTOR_FNS_
# define MCLIB_STR_FORMAT_VECTOR_FNS_

static inline _str_arg_t _sarg_vec2(vec2 v) {
  _str_arg_t ret = { .type = _str_arg_vec2 };
  assert(sizeof(ret.other) >= sizeof(v));
  *((vec2*)ret.other) = v;
  return ret;
}

static inline _str_arg_t _sarg_vec3(vec3 v) {
  _str_arg_t ret = { .type = _str_arg_vec3 };
  assert(sizeof(ret.other) >= sizeof(v));
  *((vec3*)ret.other) = v;
  return ret;
}

static inline _str_arg_t _sarg_vec4(vec4 v) {
  _str_arg_t ret = { .type = _str_arg_vec4 };
  assert(sizeof(ret.other) >= sizeof(v));
  *((vec4*)ret.other) = v;
  return ret;
}

static inline _str_arg_t _sarg_vec2i(vec2i v) {
  _str_arg_t ret = { .type = _str_arg_vec2i };
  assert(sizeof(ret.other) >= sizeof(v));
  *((vec2i*)ret.other) = v;
  return ret;
}

static inline _str_arg_t _sarg_vec3i(vec3i v) {
  _str_arg_t ret = { .type = _str_arg_vec3i };
  assert(sizeof(ret.other) >= sizeof(v));
  *((vec3i*)ret.other) = v;
  return ret;
}

# endif

# define MCLIB_STR_FORMAT_VECTOR_ ,     \
    vec2:  _sarg_vec2,                  \
    vec3:  _sarg_vec3,                  \
    vec4:  _sarg_vec4,                  \
    vec2i: _sarg_vec2i,                 \
    vec3i: _sarg_vec3i                  //

#endif

////////////////////////////////////////////////////////////////////////////////
// Type selector for format args
////////////////////////////////////////////////////////////////////////////////

#ifdef _sfa
# undef _sfa
#endif

// \brief str_format argument macro
#define _sfa(arg) _Generic((arg),       \
  slice_t:            _sarg_slice,      \
  String:             _sarg_str,        \
  slice_t*:           _sarg_slice_ptr,  \
  const slice_t*:     _sarg_slice_ptr,  \
  char*:              _sarg_c_str,      \
  const char*:        _sarg_c_str,      \
  int:                _sarg_int,        \
  long long:          _sarg_int,        \
  unsigned int:       _sarg_unsigned,   \
  unsigned long:      _sarg_unsigned,   \
  unsigned long long: _sarg_unsigned,   \
  double:             _sarg_float       \
  MCLIB_STR_FORMAT_VECTOR_              \
)(arg)                                  //

#endif

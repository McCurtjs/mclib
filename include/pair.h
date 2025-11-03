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

#ifndef MCLIB_PAIR_H_
#define MCLIB_PAIR_H_

#include "types.h"

//
// Pair tuple type
//
// Creates a pair tuple struct based on the defined preprocessor state.
//
// Note: there are two "modes" for using this header - first is the standard for
//    containers using `con_type`, and the second is an override that's used to
//    avoid conflict when other containers are being defined using `con_type`.
//
// inputs:
//  - con_type: the type of the left/right pair members.
//  - con_prefix: overrides the naming of the type (avoid extra `_t` in names).
//
// override:
//  - tuple_type: the type of the left/right pair members.
//  - tuple_pair_type: optionally overrides the name of the resulting struct.
//

#define pair_deconstruct(LEFT, RIGHT, PAIR)   \
  LEFT = (PAIR).left, RIGHT = (PAIR).right    //

#endif

#if defined(tuple_type) || defined(con_type)

////////////////////////////////////////////////////////////////////////////////
// Specialized pair type (if no override is specified)
////////////////////////////////////////////////////////////////////////////////

#ifdef tuple_type
# define _tuple_type tuple_type
# ifdef tuple_pair_type
#   define _tuple_pair_type tuple_pair_type
# else
#   define _tuple_pair_type MACRO_CONCAT(pair_, _tuple_type)
# endif
#elif defined(con_type)
# define _tuple_type con_type
# ifdef con_prefix
#   define _tuple_pair_type MACRO_CONCAT3(pair_, con_prefix, _t)
# else
#   define _tuple_pair_type MACRO_CONCAT3(pair_, con_type, _t)
# endif
#endif

typedef struct _tuple_pair_type {
  union {
    _tuple_type begin[2];
    struct {
      _tuple_type left;
      _tuple_type right;
    };
  };
} _tuple_pair_type;

# undef _tuple_type
# undef _tuple_pair_type

#endif

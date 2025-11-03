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

#ifndef MCLIB_PARTITION_H_
#define MCLIB_PARTITION_H_

#include "types.h"

//
// Partition type
//
// Creates a triad struct based on the defined preprocessor state.
//
// The partition represents two halves of some object that has been split, with
//    the delimiter representing the point at which they were joined.
//
// Note: there are two "modes" for using this header - first is the standard for
//    containers using `con_type`, and the second is an override that's used to
//    avoid conflict when other containers are being defined using `con_type`.
//
// inputs:
//  - delim_type: the type of the delimiter if it's different from the L/R type.
//  - tuple_pair_type: if set, adds the pair as a union with the L/R values.
//
//  - con_type: the type of the left/right pair members.
//  - con_prefix: overrides the naming of the type (avoid extra `_t` in names).
//
// overrides:
//  - tuple_type: the type of the left/right pair members.
//  - tuple_partition_type: optionally overrides the struct name.
//

#endif

#if defined(tuple_type) || defined(con_type)

////////////////////////////////////////////////////////////////////////////////
// Specialized partition type (if no override is specified)
////////////////////////////////////////////////////////////////////////////////

#ifdef tuple_type
# define _tuple_type tuple_type
# ifdef tuple_partition_type
#   define _tuple_partition_type tuple_partition_type
# else
#   define _tuple_partition_type MACRO_CONCAT(partition_, _tuple_type)
# endif
#elif defined(con_type)
# define _tuple_type con_type
# ifdef con_prefix
#   define _tuple_partition_type MACRO_CONCAT3(partition_, con_prefix, _t)
# else
#   define _tuple_partition_type MACRO_CONCAT3(partition_, con_type, _t)
# endif
#endif

#ifdef delim_type
# define _tuple_delim_type delim_type
#else
# define _tuple_delim_type _tuple_type
#endif

typedef struct _tuple_partition_type {
# ifdef tuple_pair_type
  union {
    tuple_pair_type pair;
    struct {
# endif
      _tuple_type left;
      _tuple_type right;
# ifdef tuple_pair_type
    };
  };
# endif
  _tuple_delim_type delimiter;
} _tuple_partition_type;

# undef _tuple_delim_type
# undef _tuple_type
# undef _tuple_partition_type

#endif

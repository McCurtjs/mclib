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

#ifndef MCLIB_TYPES_H_
#define MCLIB_TYPES_H_

#ifdef __WASM__
# ifndef __DEFINED_size_t
#  define __DEFINED_size_t
typedef unsigned long size_t;
typedef          long ptrdiff_t;
# endif


# ifndef __has_builtin
#  define __has_builtin(x) 0
# endif

// Provide assert that works with wasm...
# ifndef assert
#	 if __has_builtin(__builtin_trap)
#   define assert(CONDITION) (!(CONDITION) ? __builtin_trap() : 0);
#  else
#   define assert(CONDITION)
#  endif
# endif
#else
# include <corecrt.h>
# include <assert.h>
#endif

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned short u16;
typedef unsigned char byte;
typedef size_t    jshandle;
typedef ptrdiff_t index_s;
typedef index_s   index_t;

// Shouldn't be necessary with C23?
#include <stdbool.h>

#ifndef NULL
# define NULL nullptr
#endif

#ifndef TRUE
# define TRUE true
# define FALSE false
#endif

#ifndef SQRT2
# define SQRT2 1.41421356237
#endif

#ifndef PI
# define PI 3.14159265358979323846264338f
#endif

#ifndef TAU
# define TAU (2 * PI)
#endif

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// Converts from degrees into radians
#define d2r(DEG)  ((DEG) * PI / 180.0f)

// True if an integer value is a power of 2
#define isPow2(n) ((n & (n-1)) == 0)

// Allows writing loops in the form:
//    loop {
//      // setup that happens on every iteration but always at least once
//      until (condition);
//      // do stuff each iteration after the conditional check
//    }
#ifndef loop
# define loop for(;;)
# define until(condition) if (condition) break;
#endif

// Ruby, lol
#ifndef unless
# define unless(condition) if (!(condition))
#endif

#ifndef MACRO_CONCAT
// Macro to concatinate preprocessor symbols
# define MACRO_CONCAT_RECUR(X, Y) X ## Y
# define MACRO_CONCAT(A, B) MACRO_CONCAT_RECUR(A, B)
#endif

#ifndef MACRO_CONCAT3
// Separating the additional concats so they aren't removed by the CSpec include
# define MACRO_CONCAT3(A, B, C) MACRO_CONCAT(A, MACRO_CONCAT_RECUR(B, C))
# define MACRO_CONCAT4(A, B, C, D) MACRO_CONCAT(A, MACRO_CONCAT3(B, C, D))
# define MACRO_CONCAT5(A, B, C, D, E) MACRO_CONCAT(A, MACRO_CONCAT4(B, C, D, E))
#endif

// Macro-composer that calls a given macro with a given list of arguments
#ifndef MCOMP
# define MCOMP(F, A) F A
#endif

// Stringifies a macro argument (ie: `STR(xyz)` becomes `"xyz"`
#ifndef STR
# define STR_RECUR(S) #S
# define STR(S) STR_RECUR(S)
#endif

// Used to count the number of items in an array
#ifndef ARRAY_COUNT
# define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(*arr))
#endif

// Variadic expander to apply a macro/function to each argument such as a
//		_Generic selector for type safety.
#define _va_exp_1(F,a,...) F(a)
#define _va_exp_2(F,a,...) F(a), _va_exp_1(F,__VA_ARGS__)
#define _va_exp_3(F,a,...) F(a), _va_exp_2(F,__VA_ARGS__)
#define _va_exp_4(F,a,...) F(a), _va_exp_3(F,__VA_ARGS__)
#define _va_exp_5(F,a,...) F(a), _va_exp_4(F,__VA_ARGS__)
#define _va_exp_6(F,a,...) F(a), _va_exp_5(F,__VA_ARGS__)
#define _va_exp_7(F,a,...) F(a), _va_exp_6(F,__VA_ARGS__)
#define _va_exp_8(F,a,...) F(a), _va_exp_7(F,__VA_ARGS__)
#define _va_exp_9(F,a,...) F(a), _va_exp_8(F,__VA_ARGS__)
#define _va_exp_a(F,a,...) F(a), _va_exp_9(F,__VA_ARGS__)
#define _va_exp_b(F,a,...) F(a), _va_exp_a(F,__VA_ARGS__)
#define _va_exp_c(F,a,...) F(a), _va_exp_b(F,__VA_ARGS__)
#define _va_exp_d(F,a,...) F(a), _va_exp_c(F,__VA_ARGS__)
#define _va_exp_e(F,a,...) F(a), _va_exp_d(F,__VA_ARGS__)
#define _va_exp_f(F,a,...) F(a), _va_exp_e(F,__VA_ARGS__)
#define _va_exp_g(F,a,...) F(a), _va_exp_f(F,__VA_ARGS__)
#define _va_exp_h(F,a,...) F(a), _va_exp_g(F,__VA_ARGS__)
#define _va_exp_i(F,a,...) F(a), _va_exp_h(F,__VA_ARGS__)
#define _va_exp_j(F,a,...) F(a), _va_exp_i(F,__VA_ARGS__)
#define _va_exp_k(F,a,...) F(a), _va_exp_j(F,__VA_ARGS__)
#define _va_exp_l(F,a,...) F(a), _va_exp_k(F,__VA_ARGS__)
#define _va_exp_m(F,a,...) F(a), _va_exp_l(F,__VA_ARGS__)
#define _va_exp_va(F,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,X,...)       \
        _va_exp_##X(F,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v)            //

#define _va_exp(F, ...)                                                       \
        _va_exp_va(F,__VA_ARGS__,m,l,k,j,i,h,g,f,e,d,c,b,a,9,8,7,6,5,4,3,2,1) //

// Squelches warnings about unused parameters.
// Ideally, for GCC and Clang this should be __attribute__((unused)) in the
//    function declaration, but there's no equivalent for MSVC that would work
//    there. C++ supports omitting the name, but C does not. Casting the param
//    to void in the function body works on all three with max level warnings.
#define PARAM_UNUSED(PARAM) (void)PARAM;

#ifdef _MSC_VER
#
# // Visual Studio doesn't have an explicit fallthrough specifier
# define SWITCH_FALLTHROUGH
#
# // Annoyingly, MSVC for some reason detects the _Generic specifier as "unused".
# pragma warning ( disable : 4189 ) // local initialized but not referenced
#
// Specifier to indicate an intentional fallthrough on a switch case
#else
#
# // Specifier for intentional fallthrough on switch case statements
# define SWITCH_FALLTHROUGH __attribute__((fallthrough))
#
#endif

#endif

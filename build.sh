#!/bin/bash

build_target="clang"
unit_test=false
build_type="Debug"
skip_cmake=false
args=""
read_args=true

while [ "$read_args" == true ] && [ "$1" != "" ]; do
  case "$1" in
    -t | --target)
      build_target="$2"
      shift 1
      ;;
    -h | --help)
      echo ": - -- Options"
      echo ": h help                                 : prints this message"
      echo ": t target  [clang|gcc|mingw|msvc]       : sets build target"
      echo ": r release                              : release build (default is debug)"
      echo ": a args    \" \"                          : passes args to built exe (if any)"
      echo ": s skip-cmake                           : skips cmake"
      exit
      ;;
    -- )
      args="$@"
      read_args=false
      ;;
    -r | --release)
      build_type="Release"
      ;;
    -s | --skip-cmake)
      skip_cmake=true
      ;;
    *)
      echo ": Unknown parameter: $1"
      exit
      ;;
  esac
  shift 1
done

# Get make executable
make_exe="no_make"

which make &> /dev/null
if [ "$?" == "0" ]; then
  make_exe="make"
else
  which mingw32-make &> /dev/null
  if [ "$?" == "0" ]; then
    make_exe="mingw32-make"
  fi
fi

# executable checks
case "$build_target" in
  "wasm" )
    which clang &> /dev/null
    if [ "$?" != "0" ]; then
      echo ": build $build_target: clang compiler not found, exiting build."
      exit
    fi
    ;;&
  "mingw" | "msvc" )
    which cmake &> /dev/null
    if [ "$?" != "0" ]; then
      echo ": build $build_target: cmake not found, exiting build."
      exit
    fi
    ;;&
  "mingw" )
    if [ "$make_exe" == "no_make" ]; then
      echo ": build $build_target: make not found, exiting build."
      exit
    fi
    ;;
esac

# Run build based on target type

sources_test=" \
  ./lib/cspec/cspec.c \
  ./lib/cspec/tst/cspec_spec.c \
  ./tst/spec_main.c \
  ./tst/span_spec.c \
  ./tst/slice_spec.c \
  ./tst/str_spec.c \
"

sources=" \
  ./src/span.c \
  ./src/slice.c \
  ./src/array.c \
  ./src/str.c \
  ./src/utility.c \
"

includes=" \
  -I ./include -I ./lib/cspec \
"

flags_memtest=" \
  -Dmalloc=cspec_malloc -Drealloc=cspec_realloc \
  -Dcalloc=cspec_calloc -Dfree=cspec_free \
";

# WASM not supported here
if [ "$build_target" = "wasm" ]; then

  echo ": WASM build not supported for McLib tests (requires libc)"

# Clang build targets
elif [ "$build_target" = "clang" ]; then

  flags_common="-Wall -Wextra -Wno-missing-braces"

  flags_debug_opt="-g -O0"
  if [ "$build_type" = "Release" ]; then
    flags_debug_opt="-Oz" # -flto causes a link issue in clang?
  fi

  mkdir -p build/$build_target/$build_type

  clang $flags_memtest -o build/clang/$build_type/test.exe \
    $flags_common $flags_debug_opt $includes $sources $sources_test

  if [ "$?" == "0" ]; then
    ./build/clang/$build_type/test.exe $args
  fi

# GCC
elif [ "$build_target" = "gcc" ]; then

  mkdir -p build/gcc/$build_type

  gcc -o build/gcc/$build_type/test.exe $flags_memtest $includes $sources $sources_test

  if [ "$?" == "0" ]; then
    ./build/gcc/$build_type/test.exe $args
  fi

# CMake MinGW on Windows with GCC
elif [ "$build_target" = "mingw" ]; then

  if [ "$skip_cmake" != true ]; then
    cmake -G "MinGW Makefiles" -S . -B build/mingw/$build_type -DCMAKE_BUILD_TYPE=$build_type
    if [ "$?" != "0" ]; then
      exit
    fi
  fi

  pushd . &> /dev/null
  cd build/mingw/$build_type
  eval $make_exe
  if [ "$?" == "0" ]; then
    ./McLib_specs.exe $args
  fi
  popd &> /dev/null

# CMake MSVC
elif [ "$build_target" = "msvc" ]; then

  cmake -G "Visual Studio 17 2022" -S . -B build/msvc
  if [ "$?" != "0" ]; then
    exit
  fi

# No matching build types
else

  echo ": Invalid build target: $build_target"

fi

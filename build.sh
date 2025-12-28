#!/bin/bash

build_target="clang"
unit_test=false
build_type="Debug"
skip_cmake=false
clean=false
clean_only=false
args=""
open=false

while [ "$1" != "" ]; do
  case "$1" in
    -h | --help)
      echo ": - -- Options"
      echo ": h help                                 : prints this message"
      echo ": t target  [clang|gcc|mingw|msvc]       : sets build target"
      echo ": c clean                                : cleans build files for target"
      echo ":   clean-only                           : cleans files for target then exits"
      echo ": r release                              : release build (default is debug)"
      echo ": s skip-cmake                           : skips cmake"
      echo ": u update                               : update submodules"
      echo ": p pull                                 : pull with submodules"
      echo ": o open                                 : opens the IDE for msvc target"
      echo ": -- <args>                              : passes remaining args to built exe (if any)"
      exit
      ;;
    -t | --target)
      build_target="$2"
      shift 1
      ;;
    -- )
      args="$@"
      break 1
      ;;
    -c | --clean)
      clean=true
      clean_only=true
      ;;
    --rebuild)
      clean=true
      ;;
    -r | --release)
      build_type="Release"
      ;;
    -E | --preprocessed)
      build_type="Preprocess"
      args="$2"
      shift 1
      ;;
    -s | --skip-cmake)
      skip_cmake=true
      ;;
    -u | --update)
      git submodule update --init --remote # --recursive
      echo ": Updating with submodules"
      build_target="none"
      ;;
    -p | --pull)
      git pull --recurse-submodules
      echo ": Pulling with submodules"
      build_target="none"
      ;;
    -o | --open)
      open=true
      ;;
    *)
      echo ": Unknown parameter: $1"
      exit
      ;;
  esac
  shift 1
done

# Clean files from build target
if [ $clean = true ]; then
  echo ": Deleting build files for $build_target"
  rm -rf ./build/$build_target
  if [ $clean_only = true ]; then
    exit
  fi
  echo ": Continuing build..."
fi

if [ "$build_target" == "none" ]; then
  exit
fi

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

sources_test="
  ./lib/cspec/cspec.c
  ./lib/cspec/tst/cspec_spec.c
  ./tst/*.c
"

sources="
  ./src/*.c
  ./lib/murmur3/murmur3.c
"

includes=" \
  -I ./include -I ./lib/cspec -I ./lib/murmur3 \
"

flags_memtest=" \
  -Dmalloc=cspec_malloc -Drealloc=cspec_realloc \
  -Dcalloc=cspec_calloc -Dfree=cspec_free \
  -Dassert=cspec_assert \
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

  clang -std=c2x $flags_memtest -o build/clang/$build_type/test.exe \
    $flags_common $flags_debug_opt $includes $sources $sources_test

  if [ "$?" == "0" ]; then
    ./build/clang/$build_type/test.exe $args
  fi

# GCC
elif [ "$build_target" = "gcc" ]; then

  mkdir -p build/gcc/$build_type

  if [ "$build_type" == "Preprocess" ]; then

    gcc -std=c2x -E -C src/$args.c -o build/gcc/$build_type/$args.i $includes -pedantic

  else
    gcc -std=c2x -o build/gcc/$build_type/test.exe \
      $flags_memtest $includes $sources $sources_test -pedantic

    if [ "$?" == "0" ]; then
      ./build/gcc/$build_type/test.exe $args
    fi
  fi

# CMake MinGW on Windows with GCC
elif [ "$build_target" = "mingw" ]; then

  if [ "$skip_cmake" != true ]; then
    cmake -G "MinGW Makefiles" -S . -B build/mingw/$build_type \
      -DCSPEC_MEMTEST=ON -DCSPEC_ASSERT=ON
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

  cmake -G "Visual Studio 17 2022" -S . -B build/msvc \
    -DCSPEC_MEMTEST=ON -DCSPEC_ASSERT=ON

  if [ "$?" == "0" ] && [ "$open" == true ]; then
    echo ": Opening Visual Studio"
    start ./build/msvc/McLib_specs.sln
  fi

# No matching build types
else

  echo ": Invalid build target: $build_target"

fi

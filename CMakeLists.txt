option(CSPEC_MEMTEST "Enable memory testing defines for spec build" OFF)
option(CSPEC_ASSERT "Enable assert override for spec build" OFF)

add_library(McLib)
set_property(TARGET McLib PROPERTY C_STANDARD 23)
target_include_directories(McLib PUBLIC include)
target_sources(McLib PRIVATE
  src/array.c
  src/map.c
  src/mat.c
  src/quat.c
  src/slice.c
  src/span.c
  src/str.c
  src/str_format.c
  src/utility.c
  src/vec.c
  src/view.c
  include/types.h
  include/array.h
  include/array_byte.h
  include/array_slice.h
  include/map.h
  include/mat.h
  include/pair.h
  include/partition.h
  include/quat.h
  include/slice.h
  include/span.h
  include/span_base.h
  include/span_byte.h
  include/span_slice.h
  include/str.h
  include/str_format.h
  include/utility.h
  include/vec.h
  include/view.h
)

# Include Murmur3 library for default hashing solution
add_library(Murmur3)
set_property(TARGET Murmur3 PROPERTY C_STANDARD 23)
target_include_directories(Murmur3 PUBLIC lib/murmur3)
target_sources(Murmur3 PRIVATE
  lib/murmur3/murmur3.c
  lib/murmur3/murmur3.h
)

target_link_libraries(McLib PUBLIC Murmur3)

if (WIN32)
  target_compile_definitions(McLib PUBLIC
    _CRT_SECURE_NO_WARNINGS
  )
endif()

# If building as a standalone, create the example project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.6)
  project(McLib_specs)
  set(MCLIB_TARGET McLib_specs)
  add_executable(McLib_specs)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT McLib_specs)
  set_property(TARGET ${MCLIB_TARGET} PROPERTY C_STANDARD 23)

  # Include CSpec to drive testing
  add_subdirectory(lib/cspec)

  # Include spec sources
  target_sources(McLib_specs PRIVATE
    lib/cspec/tst/cspec_spec.c
    tst/spec_main.c
    tst/array_spec.c
    tst/span_spec.c
    tst/str_spec.c
    tst/slice_spec.c
    tst/map_spec.c
    tst/vec_spec.c
  )

  target_link_libraries(${MCLIB_TARGET} PRIVATE CSpec)
  target_link_libraries(${MCLIB_TARGET} PRIVATE McLib)

  if (MSVC)
    target_compile_options(${MCLIB_TARGET} PRIVATE /W4 /WX /std:clatest)
    target_link_libraries(${PROJECT_NAME} PRIVATE DbgHelp)

    # Add a C++ project in MSVC for natvis support
    add_library(Natvis_injector)
    target_sources(Natvis_injector PRIVATE McLib.natvis)
    set_target_properties(Natvis_injector PROPERTIES LINKER_LANGUAGE CXX)

    # Testbed project for checking Natvis output
    add_executable(Natvis_test)
    set_property(TARGET Natvis_test PROPERTY C_STANDARD 23)
    target_sources(Natvis_test PRIVATE tst/natvis.c)
    target_link_libraries(Natvis_test PRIVATE McLib)
    target_link_libraries(Natvis_test PRIVATE CSpec)
    target_link_libraries(Natvis_test PRIVATE DbgHelp)
    target_compile_options(Natvis_test PRIVATE /std:clatest)
  else()
    target_compile_options(${MCLIB_TARGET} PRIVATE
      -std=c2x -Wall -Wextra -Wpedantic -Werror -Wno-unused-value
    )
  endif()
endif()

# Build library for specs
if(CSPEC_MEMTEST STREQUAL ON)
  target_compile_definitions(McLib PUBLIC ${CSPEC_MEMTEST_DEFINES})
endif()

if(CSPEC_ASSERT STREQUAL ON)
  target_compile_definitions(McLib PUBLIC ${CSPEC_ASSERT_DEFINE})
endif()

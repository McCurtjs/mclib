option(CSPEC_MEMTEST "Enable memory testing defines for spec build" OFF)

add_library(McLib)
target_include_directories(McLib PUBLIC ./include)
target_sources(McLib PRIVATE
  src/utility.c
  src/array.c
  src/mat.c
  src/str.c
  src/vec.c
)

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

  # Include CSpec to drive testing
  set(CSPEC_MEMTEST ON)
  add_subdirectory(lib/cspec)

  # Include spec sources
  target_sources(McLib_specs PRIVATE
    lib/cspec/tst/cspec_spec.c
    tst/spec_main.c
    tst/str_spec.c
  )

  target_link_libraries(${MCLIB_TARGET} PRIVATE CSpec)
  target_link_libraries(${MCLIB_TARGET} PRIVATE McLib)

  if (MSVC)
    target_compile_options(${MCLIB_TARGET} PRIVATE /W4 /WX /std:clatest)
  else()
    target_compile_options(${MCLIB_TARGET} PRIVATE
      -Wall -Wextra -Wpedantic -Werror
    )
  endif()
endif()

# Build library for specs
if(CSPEC_MEMTEST STREQUAL ON)
  target_compile_definitions(McLib PUBLIC ${CSPEC_MEMTEST_DEFINES})
endif()

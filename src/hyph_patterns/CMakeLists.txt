
cmake_minimum_required(VERSION 3.10)

project(pattern_setup
  LANGUAGES C
  VERSION 0.1.0)

SET(BINARY_NAME hyph_pattern_setup)
SET(LIBRARY_VERSION_MAJOR 0)
SET(LIBRARY_VERSION_STRING 1.0)

set(pattern_setup_SOURCES
  ../tools/list.c
  ../tools/z_ucs.c
  ../tools/filesys_c.c
  ../tools/filesys.c
  pattern_setup.c
  hyph_pattern_tools.c)

add_executable(pattern_setup ${pattern_setup_SOURCES})

# targets without output will always be built
add_custom_target(dummy_target ALL
  DEPENDS ../hyph_patterns.c
  )

file(GLOB hyph_pattern_sources ??_??.txt)

# rebuild "../hyph_patterns.c" on demand
add_custom_command(
  OUTPUT ../hyph_patterns.c
  COMMAND pattern_setup
  WORKING_DIRECTORY ..
  DEPENDS ${hyph_pattern_sources}
  COMMENT "Compiling hyphenation patterns"
  VERBATIM
  )


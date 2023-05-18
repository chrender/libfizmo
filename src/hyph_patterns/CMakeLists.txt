
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

add_custom_command(
  TARGET pattern_setup
  POST_BUILD
  COMMAND pattern_setup
  WORKING_DIRECTORY ..
  DEPENDS ../de_DE.txt ../en_GB.txt ../fr_FR.txt
  COMMENT "Compiling hyphenation patterns"
  VERBATIM
  )


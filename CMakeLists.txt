
cmake_minimum_required(VERSION 3.10)

include(ExternalProject)

#set(CMAKE_BUILD_TYPE Debug)

project(libfizmo
  LANGUAGES C
  HOMEPAGE_URL https://fizmo.spellbreaker.org
  DESCRIPTION "fizmo interpreter core library"
  VERSION 0.8.0)

ExternalProject_Add(hyph_pattern_preparation
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/hyph_patterns
  BINARY_DIR ${CMAKE_BINARY_DIR}/hyph_patterns_build
  CMAKE_ARGS
  ${GLOBAL_DEFAULT_ARGS}
  ${GLOBAL_THIRDPARTY_LIB_ARGS}
  BUILD_COMMAND make
  INSTALL_COMMAND cmake -E echo "Skipping install step."
  )

ExternalProject_Add(locale_data_preparation
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/locales
  BINARY_DIR ${CMAKE_BINARY_DIR}/loacles_build
  CMAKE_ARGS
  ${GLOBAL_DEFAULT_ARGS}
  ${GLOBAL_THIRDPARTY_LIB_ARGS}
  BUILD_COMMAND make
  INSTALL_COMMAND cmake -E echo "Skipping install step."
  )

option(ENABLE_TRACING "Enable tracing" OFF)
if (ENABLE_TRACING)
  add_definitions(-DENABLE_TRACING)
endif()

option(ENABLE_BABEL "Enable babel" ON)
if (ENABLE_BABEL)
  add_definitions(-DENABLE_BABEL)
  find_package(LibXml2 REQUIRED)
  set(pc_req_public "libxml-2.0")
else()
  set(pc_req_public)
endif()

option(ENABLE_STRICT_Z "Enable strict Z-machine behavior" OFF)
if (ENABLE_STRICT_Z)
  add_definitions(-DENABLE_STRICT_Z)
endif()

option(ENABLE_SEGFAULT_ON_ERROR "Enable segfault on error" OFF)
if (ENABLE_SEGFAULT_ON_ERROR)
  add_definitions(-DENABLE_SEGFAULT_ON_ERROR)
endif()

option(ENABLE_FILELIST "Enable filelist" ON)
if (ENABLE_FILELIST)
  add_definitions(-DENABLE_FILELIST)
endif()

option(ENABLE_BLOCKBUFFER "Enable blockbuffer" ON)
if (ENABLE_BLOCKBUFFER)
  add_definitions(-DENABLE_BLOCKBUFFER)
endif()

option(ENABLE_COMMAND_HISTORY "Enable command history" ON)
if (ENABLE_COMMAND_HISTORY)
  add_definitions(-DENABLE_COMMAND_HISTORY)
endif()

option(ENABLE_OUTPUT_HISTORY "Enable output history" ON)
if (ENABLE_OUTPUT_HISTORY)
  add_definitions(-DENABLE_OUTPUT_HISTORY)
endif()

option(ENABLE_CONFIG_FILES "Enable config files" ON)
if (ENABLE_CONFIG_FILES)
  add_definitions(-DENABLE_CONFIG_FILES)
endif()

option(ENABLE_PREFIX_COMMANDS "Enable prefix commands" ON)
if (ENABLE_PREFIX_COMMANDS)
  add_definitions(-DENABLE_PREFIX_COMMANDS)
endif()

option(ENABLE_DEBUGGER "Enable debugger" OFF)
if (ENABLE_DEBUGGER)
  add_definitions(-DENABLE_DEBUGGER)
endif()

option(FIZMO_DIST_VERSION "Set fizmo-dist version" OFF)
if (FIZMO_DIST_VERSION)
  add_definitions(-DFIZMO_DIST_VERSION=${FIZMO_DIST_VERSION})
endif()

set (MyCSources
  src/interpreter/babel.c
  src/interpreter/blockbuf.c
  src/interpreter/blorb.c
  src/interpreter/cmd_hst.c
  src/interpreter/config.c
  src/interpreter/debugger.c
  src/interpreter/filelist.c
  src/interpreter/fizmo.c
  src/interpreter/history.c
  src/interpreter/hyphenation.c
  src/interpreter/iff.c
  src/interpreter/mathemat.c
  src/interpreter/misc.c
  src/interpreter/mt19937ar.c
  src/interpreter/object.c
  src/interpreter/output.c
  src/interpreter/property.c
  src/interpreter/routine.c
  src/interpreter/savegame.c
  src/interpreter/sound.c
  src/interpreter/stack.c
  src/interpreter/streams.c
  src/interpreter/table.c
  src/interpreter/text.c
  src/interpreter/undo.c
  src/interpreter/variable.c
  src/interpreter/wordwrap.c
  src/interpreter/zpu.c
  src/tools/filesys.c
  src/tools/filesys_c.c
  src/tools/i18n.c
  src/tools/list.c
  src/tools/stringmap.c
  src/tools/tracelog.c
  src/tools/types.c
  src/tools/z_ucs.c
  src/hyph_patterns/hyph_pattern_tools.c
  src/hyph_patterns/hyph_patterns.c
  src/locales/libfizmo_locales.c
  src/locales/locale_data.c
  src/locales/locale_data.h)

add_library(fizmo ${MyCSources})

install(TARGETS fizmo)
# PUBLIC_HEADER cannot be used for TARGETS fizmo, since it doesn't keep
# the directory tree and installs all *.h flat into "include/". So:

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" # source directory
  DESTINATION "include" # target directory
  FILES_MATCHING # install only matched files
  PATTERN "*.h" # select header files
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
  DESTINATION "lib/pkgconfig")

set(pc_libs_private)
set(pc_req_private)
configure_file(src/libfizmo.pc.in libfizmo.pc @ONLY)


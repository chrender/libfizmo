
cmake_minimum_required(VERSION 3.10)

include(ExternalProject)

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

SET(BINARY_NAME fizmo)
SET(LIBRARY_VERSION_MAJOR 0)
SET(LIBRARY_VERSION_STRING 8.0)

option(ENABLE_TRACING "Enable tracing" OFF)
#set(FIZMO_DIST_VERSION "" CACHE STRING "Fizmo's dist version")
if (NOT(DISABLE_BABEL))
  #FindLibXml2
  find_package(LibXml2 REQUIRED)
  #CPPFLAGS += -DUSE_LIBXML2
  #CFLAGS += "-I${LIBXML2_INCLUDE_DIR}"
  #LDADD += "-L${LIBXML2_LIBRARY} -l${LIBXML2_LIBRARIES}"
  set(pc_req_public "libxml-2.0")
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
  src/locales/locale_data.c)
add_library (fizmo ${MyCSources})

SET_TARGET_PROPERTIES (
	${BINARY_NAME} PROPERTIES
	VERSION		${LIBRARY_VERSION_STRING}
	SOVERSION	${LIBRARY_VERSION_MAJOR})

install(TARGETS fizmo)
# PUBLIC_HEADER cannot be used for TARGETS fizmo, since it doesn't keep
# the directory tree and installs all *.h flat into "include/". So:

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/" # source directory
        DESTINATION "include" # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# https://www.scivision.dev/cmake-generate-pkg-config/

set(target1 ${BINARY_NAME})
set(pc_libs_private)
set(pc_req_private)
#set(pc_req_public)
configure_file(src/libfizmo.pc.in libfizmo.pc @ONLY)


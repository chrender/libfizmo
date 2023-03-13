
cmake_minimum_required(VERSION 3.10)

project(libfizmo
LANGUAGES C
HOMEPAGE_URL https://fizmo.spellbreaker.org
DESCRIPTION "fizmo interpreter core library"
VERSION 0.8.0)

include(GNUInstallDirs)

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

file(GLOB_RECURSE MyCSources "*.c")
file(GLOB ${PROJECT_NAME}_headers ${PROJECT_SOURCE_DIR}/src/interpreter/*.h)
#message(${${PROJECT_NAME}_headers})
#message("${MyCSources}")
list(FILTER MyCSources EXCLUDE REGEX "src/tools/tools_test.c")
list(FILTER MyCSources EXCLUDE REGEX "src/hyph_patterns/pattern_setup.c")
add_library (fizmo ${MyCSources})
#add_dependencies(fizmo src/hyph_patterns/hyph_patterns.c)

SET_TARGET_PROPERTIES (
	${BINARY_NAME} PROPERTIES
	VERSION		${LIBRARY_VERSION_STRING}
	SOVERSION	${LIBRARY_VERSION_MAJOR})
      #PUBLIC_HEADER  "${libfizmo_headers}")

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


#add_executable(pattern_setup
# src/hyph_patterns/pattern_setup.c
# src/hyph_patterns/hyph_pattern_tools.c
# src/tools/list.c
# src/tools/z_ucs.c
# src/tools/filesys_c.c
# src/tools/filesys.c)

#set_target_properties(pattern_setup
#    PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY "src/hyph_patterns"
#)

#add_custom_command(
#  OUTPUT src/hyph_patterns/hyph_patterns.c
#  COMMAND src/hyph_patterns/pattern_setup
#  DEPENDS src/hyph_patterns/de_DE.txt src/hyph_patterns/en_GB.txt src/hyph_patterns/fr_FR.txt
#  COMMENT "Preparing hyphenation patterns"
#  VERBATIM
#)

#add_custom_target(
#  HyphenationPatternPreparation ALL
#  DEPENDS src/hyph_patterns/hyph_patterns.c
#)


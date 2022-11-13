
cmake_minimum_required(VERSION 3.10)

project(libfizmo
LANGUAGES C
HOMEPAGE_URL https://fizmo.spellbreaker.org
DESCRIPTION "fizmo interpreter core library"
VERSION 0.8.0)

SET(BINARY_NAME fizmo)
SET(LIBRARY_VERSION_MAJOR 0)
SET(LIBRARY_VERSION_STRING 8.0)

option(ENABLE_TRACING "Enable tracing" OFF)

# And possibly other commands dealing with things
# directly in the "lx" directory

file(GLOB_RECURSE MyCSources "*.c")
file(GLOB ${PROJECT_NAME}_headers ${PROJECT_SOURCE_DIR}/interpreter/*.h)
message(${${PROJECT_NAME}_headers})
message("${MyCSources}")
list(FILTER MyCSources EXCLUDE REGEX "tools/tools_test.c")
add_library (fizmo ${MyCSources})

SET_TARGET_PROPERTIES (
	${BINARY_NAME} PROPERTIES
	VERSION		${LIBRARY_VERSION_STRING}
	SOVERSION	${LIBRARY_VERSION_MAJOR})
      #PUBLIC_HEADER  "${libfizmo_headers}")


include(GNUInstallDirs)

install(TARGETS fizmo)
# PUBLIC_HEADER cannot be used for TARGETS fizmo, since it doesn't keep
# the directory tree and installs all *.h flat into "include/". So:

install(DIRECTORY "${PROJECT_SOURCE_DIR}/" # source directory
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
set(pc_req_public)
configure_file(libfizmo.pc.in libfizmo.pc @ONLY)


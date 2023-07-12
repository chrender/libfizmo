
#[[

  src/locales/CMakeLists.txt

  This file is part of fizmo.

  ---

  Copyright (c) 2010-2023 Christoph Ender.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ---



  The main libfizmo build process

  Since the "hyph_patterns.c" has to be updated

  file has be pre-processed by the
  "pattern_setup" binary

  binary, which has to be compiled and run before the main build takes
  place, compilation and execution of the required "pattern_setup" binary
  is
  done via


]]


cmake_minimum_required(VERSION 3.10)

project(locale_setup
  LANGUAGES C
  VERSION 0.1.0)

SET(BINARY_NAME locale_setup)
SET(LIBRARY_VERSION_MAJOR 0)
SET(LIBRARY_VERSION_STRING 1.0)

set(locale_setup_SOURCES
  ../tools/z_ucs.c
  ../tools/filesys.c
  ../tools/filesys_c.c
  ../tools/list.c
  locale_setup.c)

add_executable(locale_setup ${locale_setup_SOURCES})

# targets without output will always be built
add_custom_target(dummy_target ALL
  DEPENDS _non_existent_file_forcing_custom_command_execution
    ../en_GB.txt
    ../de_DE.txt
    ../fr_FR.txt
  )

# rebuild "../locale_data.c" on demand
add_custom_command(
  OUTPUT
    _non_existent_file_forcing_custom_command_execution
    src/tools/locale_data.c
    src/tools/locale_data.h
  COMMAND locale_setup
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS ${locale_sources}
  COMMENT "Compiling locale data"
  VERBATIM
  )


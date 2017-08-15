
# configure.ac
#
# This file is part of fizmo.
#
# Copyright (c) 2009-2017 Christoph Ender.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



AC_ARG_ENABLE([tracing],
 [AS_HELP_STRING([--enable-tracing],
                 [enable tracelog for debugging])],
 [],
 [enable_tracing=no])

AC_ARG_ENABLE([babel],
 [AS_HELP_STRING([--disable-babel],
                 [disable babel functionality which means that libxml2 will \
 not be required for building])],
 [],
 [enable_babel=yes])

AC_ARG_ENABLE([strict-z],
 [AS_HELP_STRING([--enable-strict-z],
                 [enforce strict Z-Machine handling])],
 [],
 [enable_strict_z=no])

AC_ARG_ENABLE([segfault-on-error],
 [AS_HELP_STRING([--enable-segfault-on-error],
                 [deprecated debug helper])],
 [],
 [enable_segfault-on-error=no])

AC_ARG_ENABLE([filelist],
 [AS_HELP_STRING([--disable-filelist],
                 [disable story-list-keeping functionality])],
 [],
 [enable_filelist=yes])

AC_ARG_ENABLE([blockbuffer],
 [AS_HELP_STRING([--disable-blockbuffer],
                 [disable buffering ability for upper window])],
 [],
 [enable_blockbuffer=yes])

AC_ARG_ENABLE([command-history],
 [AS_HELP_STRING([--disable-command-history],
                 [disable history of previously entered commands])],
 [],
 [enable_command_history=yes])

AC_ARG_ENABLE([output-history],
 [AS_HELP_STRING([--disable-output-history],
                 [disable history of recent screen output])],
 [],
 [enable_output_history=yes])

AC_ARG_ENABLE([config-files],
 [AS_HELP_STRING([--disable-config-files],
                 [disable parsing of configuration files])],
 [],
 [enable_config_files=yes])

AC_ARG_ENABLE([prefix-commands],
 [AS_HELP_STRING([--disable-prefix-commands],
                 [disable in-game interpreter commands])],
 [],
 [enable_prefix_commands=yes])

AC_ARG_ENABLE([debugger],
 [AS_HELP_STRING([--enable-debugger],
                 [enable experimental debugging functionality])],
 [],
 [enable_debugger=no])

AC_INIT(
 [libfizmo],
 [0.7.15],
 fizmo@spellbreaker.org,
 libfizmo)

AC_CONFIG_AUX_DIR([.])

m4_include(config-libs.m4)

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

AC_SUBST([LIBFIZMO_REQS], $libfizmo_reqs)

AM_CONDITIONAL([FIZMO_DIST_BUILD], [test "x" = ""])

AM_CONDITIONAL([ENABLE_TRACING],
                [test "$enable_tracing" = "yes"])

m4_include(config-conds.m4)

AC_CONFIG_FILES([Makefile src/tools/Makefile src/interpreter/Makefile])
AC_OUTPUT


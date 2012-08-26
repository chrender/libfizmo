
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


AC_INIT([libfizmo], [0.7.4])


## Determine XML2 include path
#AC_MSG_CHECKING(for libxml/xmlmemory.h)
#
## Can we include headers using system include dirs?
#AC_TRY_COMPILE([#include <libxml/xmlmemory.h>], [int a = 1;],
#    XML2_INCLUDE=" ",
#    XML2_INCLUDE=
#)
#
## Hunt through several possible directories to find the includes for libxml2
#if test "x$XML2_INCLUDE" = "x"; then
#    old_CPPFLAGS="$CPPFLAGS"
#    for i in $xml2_include_dir /usr/include /usr/local/include /usr/include/libxml2 /usr/local/include/libxml2 ; do
#        CPPFLAGS="$old_CPPFLAGS -I$i"
#        AC_TRY_COMPILE([#include <libxml/xmlmemory.h>], [int a = 1;],
#            XML2_INCLUDE="-I$i",
#            XML2_INCLUDE=
#        )
#        if test "x$XML2_INCLUDE" != "x"; then
#            break;
#        fi
#    done
#    CPPFLAGS="$old_CPPFLAGS $XML2_INCLUDE"
#fi


AS_IF([test "x$enable_babel" != "xno"], [
  PKG_CHECK_MODULES([libxml2], [libxml-2.0])
  AC_CHECK_LIB([xml2], [xmlParseFile])
  libfizmo_reqs="libxml-2.0"
])
#AC_CHECK_LIB([xml2], [xmlParseFile], [AC_DEFINE([HAVE_LIBXML2], [1], [Define to 1 if you have the 'libxml2' library (-lxml2).]) TESTLIBS="$TESTLIBS -lxml2"])
AC_CHECK_LIB([m], [cos])

#AC_CHECK_LIB([png], [png_create_write_struct_2], [HAS_LIBPNG=1])
#AC_CHECK_LIB([Xext], [XextCreateExtension], TESTLIBS="$TESTLIBS -lXext",, $TESTLIBS)
#AC_CHECK_LIB([X11], [XOpenDisplay], TESTLIBS="$TESTLIBS -lX11",, $TESTLIBS)


AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

AM_CONDITIONAL([FIZMO_BUILD_PREFIX_EMPTY], [test "$fizmo_build_prefix" = ""])
AC_SUBST([LIBFIZMO_REQS], $libfizmo_reqs)

AM_CONDITIONAL([ENABLE_TRACING],
                [test "$enable_tracing" = "yes"])

AM_CONDITIONAL([ENABLE_BABEL],
                [test "$enable_babel" != "no"])

AM_CONDITIONAL([ENABLE_STRICT_Z],
                [test "$enable_strict_z" = "yes"])

AM_CONDITIONAL([ENABLE_SEGFAULT_ON_ERROR],
                [test "$enable_segfault-on-error" = "yes"])

AM_CONDITIONAL([ENABLE_FILELIST],
                [test "$enable_filelist" != "no"])

AM_CONDITIONAL([ENABLE_BLOCKBUFFER],
                [test "$enable_blockbuffer" != "no"])

AM_CONDITIONAL([ENABLE_COMMAND_HISTORY],
                [test "$enable_command_history" != "no"])

AM_CONDITIONAL([ENABLE_OUTPUT_HISTORY],
                [test "$enable_output_history" != "no"])

AM_CONDITIONAL([ENABLE_CONFIG_FILES],
                [test "$enable_config_files" != "no"])

AM_CONDITIONAL([ENABLE_PREFIX_COMMANDS],
                [test "$enable_prefix_commands" != "no"])

AM_CONDITIONAL([ENABLE_DEBUGGER],
                [test "$enable_debugger" = "yes"])

AC_CONFIG_FILES([Makefile src/tools/Makefile src/interpreter/Makefile])
AC_OUTPUT



AC_ARG_WITH([libxml2-includedir],
 [AS_HELP_STRING([--with-libxml2-includedir],
         [Specify include directory to use for libxml2])],
 [AC_SUBST([with_libxml2_includedir],
  [$( echo $(cd $(dirname "$with_libxml2_includedir") && pwd -P)/$(basename "$with_libxml2_includedir") )])],
 [with_libxml2_includedir=])

AC_ARG_WITH([libxml2-libdir],
 [AS_HELP_STRING([--with-libxml2-libdir],
         [Specify library directory for libxml2])],
 [AC_SUBST([with_libxml2_libdir],
  [$( echo $(cd $(dirname "$with_libxml2_libdir") && pwd -P)/$(basename "$with_libxml2_libdir") )])],
 [with_libxml2_libdir=])


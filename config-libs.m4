
AS_IF([test "x$enable_babel" != "xno"], [
  PKG_CHECK_MODULES([libxml2], [libxml-2.0])
  AC_CHECK_LIB([xml2], [xmlParseFile])
  libfizmo_reqs="libxml-2.0"
])
AC_CHECK_LIB([m], [cos])


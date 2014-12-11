
# This is included from fizmo-dist and not required by libfizmo's own
# configuration. It nevertheless needs to be maintained so fizmo-dist
# will still work.
#
# The $build_prefix, $build_prefix_cflags and $build_prefix_libs are
# pre-defined by fizmo-dist.

#libfizmo_CFLAGS="-I$build_prefix_cflags $xml2_CFLAGS $m_CFLAGS"
#libfizmo_LIBS="-L$build_prefix_libs -lfizmo $xml2_LIBS $m_LIBS"
#
#AC_SUBST([libfizmo_CFLAGS], $libfizmo_CFLAGS)
#AC_SUBST([libfizmo_LIBS], $libfizmo_LIBS)

AC_SUBST([libfizmo_CFLAGS], "-I$build_prefix_cflags $xml2_CFLAGS $m_CFLAGS")
AC_SUBST([libfizmo_LIBS], "-L$build_prefix_libs -lfizmo $xml2_LIBS $m_LIBS")


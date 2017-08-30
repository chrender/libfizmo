
AS_IF([test "x$enable_babel" != "xno"], [
  PKG_CHECK_MODULES(
    [libxml2],
    [libxml-2.0],
    [libfizmo_reqs="libxml-2.0"],
    [for dir in $with_libxml2_includedir /usr/include/libxml2 /usr/local/include/libxml2 /opt/local/include/libxml2 ; do
       AC_MSG_CHECKING(for libxml/tree.h in $dir)
       if [ test -e $dir/libxml/tree.h ]; then
         AC_MSG_RESULT(yes)
         libxml2_h_dir=$dir
         break
       else
         AC_MSG_RESULT(no)
       fi
     done
     if [ test "x$libxml2_h_dir" == "x"] ; then
       echo "Could not find libxml/tree.h."
       echo "Try setting the location using --with-libxml2-includedir."
       exit
     fi
     libxml2_nonpkg_cflags+="-I$libxml2_h_dir"
     libxml2_CFLAGS="-I$libxml2_h_dir"

     CFLAGS_SAVED=$CFLAGS
     LIBS_SAVED=$LIBS
     LDFLAGS_SAVED=$LDFLAGS
     LIBS="-lxml2"
     CFLAGS="-I$libxml2_h_dir"
     for dir in $with_libxml2_libdir /usr/lib /usr/local/lib /opt/local/lib ; do
       AC_MSG_CHECKING(for libxml2 in $dir)
       LDFLAGS="-L$dir"
       AC_TRY_LINK(
         [#include <libxml/tree.h>],
         [xmlDocPtr doc;
          doc = xmlNewDoc("1.0");],
         [AC_MSG_RESULT(yes)
          libxml2_l_dir=$dir
          break],
         [AC_MSG_RESULT(no)])
     done
     if [ test "x$libxml2_l_dir" == "x"] ; then
       echo "Could not find libxml2."
       echo "Try setting the location using --with-libxml2-libdir."
       exit
     fi
     LIBS=$LIBS_SAVED
     LDFLAGS=$LDFLAGS_SAVED
     CFLAGS=$CFLAGS_SAVED
     libxml2_nonpkg_libs="-L$libxml2_l_dir -lxml2"
     libxml2_LIBS="-L$libxml2_l_dir -lxml2"
    ])
])

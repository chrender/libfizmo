
AS_IF([test "x$enable_babel" != "xno"], [
  PKG_CHECK_MODULES([libxml2], [libxml-2.0])
  AC_CHECK_LIB([xml2], [xmlParseFile])
  libfizmo_reqs="libxml-2.0"
])

# Checking for the math library doesn't seem to make much sense since
# it appears that the math functions are often automatically included
# (which leads to something like "error: conflicting types for built-in
# function 'cos'"). Since there's probably no C installation without libm
# we can include it by default.
AS_IF([test "x$libfizmo_reqs" != "x"], [
  libfizmo_reqs+=", "
])
libfizmo_reqs+="-lm"


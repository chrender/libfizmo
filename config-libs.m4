
AS_IF([test "x$enable_babel" != "xno"], [
  PKG_CHECK_MODULES([libxml2], [libxml-2.0])
  AC_CHECK_LIB([xml2], [xmlParseFile])
  libfizmo_reqs="libxml-2.0"
])

# Checking for the math library doesn't seem to make much sense since
# it appears that the math functions are often automatically included
# (which leads to something like "error: conflicting types for built-in
# function 'cos'"). Since there's probably no C installation without libm
# we'll include it by default.
# AC_CHECK_LIB([m], [cos], m_LIBS="-lm")
m_LIBS="-lm"


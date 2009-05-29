dnl
dnl Add preprocessor symbol 
dnl
AC_DEFUN([AX_ADD_DEFINE],
[
CPPFLAGS="$CPPFLAGS -D$1"
])



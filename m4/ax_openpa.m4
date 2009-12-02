dnl 
dnl Sets OPENPA_OK and OPENPA_LDFLAGS,_LIBS,_CPPFLAGS,_CFLAGS
dnl if openpa is found.
dnl 
AC_DEFUN([AX_OPENPA],
[
AC_ARG_WITH([openpa],
  [AC_HELP_STRING([--with-openpa@<:@=ARG@:>@],
         [Use portable atomics library @<:@default=auto@:>@])],
  [openpa_dir=${withval}],
  [openpa_dir=])
OPENPA_LDFLAGS=""
OPENPA_CFLAGS=""
OPENPA_CPPFLAGS=""
OPENPA_OK=""
dnl try pkg-config
dnl AC_REQUIRE(PKG_PROG_PKG_CONFIG)
PKG_CHECK_MODULES([openpa],[openpa >= 1.0.0],[OPENPA_OK=1],[OPENPA_OK=])
if test -z $OPENPA_OK;
then
  dnl try to find the library by other means
  OLD_CPPFLAGS="$CPPFLAGS"
  OLD_LDFLAGS="$LDFLAGS"
  if test ! -z "$openpa_dir" ; then
    OPENPA_CPPFLAGS="-I${openpa_dir}/include"
    OPENPA_LDFLAGS="-L${openpa_dir}/lib"
  fi
  CPPFLAGS="$OPENPA_CPPFLAGS $CPPFLAGS"
  LDFLAGS="$OPENPA_LDFLAGS $LDFLAGS"
  AC_CHECK_HEADER([opa_primitives.h],[OPENPA_OK=1])
  CPPFLAGS="$OLD_CPPFLAGS"
  LDFLAGS="$OLD_LDFLAGS"
fi
AC_SUBST([OPENPA_LDFLAGS])
AC_SUBST([OPENPA_LIBS])
AC_SUBST([OPENPA_CPPFLAGS])
AC_SUBST([OPENPA_CFLAGS])
])

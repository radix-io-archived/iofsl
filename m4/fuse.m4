dnl run $1 if fuse is available and should be enabled
dnl otherwise run $2
dnl setfs FUSE_LDFLAGS, FUSE_CPPFLAGS, FUSE_CFLAGS
dnl
AC_DEFUN([AX_FUSE],
[
FUSE_LDFLAGS=
FUSE_CFLAGS=
FUSE_CPPFLAGS=
FUSE_LIBS=
AC_REQUIRE([AC_SYS_LARGEFILE])
PKG_CHECK_MODULES([FUSE], [fuse], [$1], [$2])
AC_SUBST([FUSE_LDFLAGS])
AC_SUBST([FUSE_CPPFLAGS])
AC_SUBST([FUSE_CFLAGS])
AC_SUBST([FUSE_LIBS])
])

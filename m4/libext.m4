dnl
dnl Find out libext (boost.m4) needs it and I don't want to
dnl go to libtool just for that
dnl

AC_DEFUN([AX_FINDLIBEXT],
[ 
AC_REQUIRE([AC_CANONICAL_SYSTEM])
AC_REQUIRE([AC_CANONICAL_HOST])
test -z "$host" && AC_MSG_ERROR([host variable is empty??])
host_os=`echo "$host" | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\3/'`
libext=a
case "$host_os" in
  cygwin* | mingw* | pw32* | cegcc*)
    libext=lib
    ;;
esac
])



#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([src/iofwd/iofwd.cpp])
AC_CONFIG_HEADER([iofwd_config.h])

AC_LANG([C++])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP

AC_REQUIRE_CPP


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT32_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])


# for fuse
dnl AC_SYS_LARGEFILE 
dnl PKG_CHECK_MODULES(FUSE, [fuse],,exit)
dnl CFLAGS="$CFLAGS $FUSE_CFLAGS"
dnl CPPFLAGS="$CPPFLAGS $FUSE_CFLAGS"
dnl LDFLAGS="$LDFLAGS $FUSE_LIBS"


AC_CONFIG_FILES([
                 Makefile
                 sandbox/module.mk
                 src/backend/module.mk
                 src/frontend/module.mk
                 src/iofwd/module.mk
                 src/iofwdutil/module.mk
                 src/metacache/module.mk
                 src/cache/module.mk
                 test/module.mk
                 ])

AC_OUTPUT

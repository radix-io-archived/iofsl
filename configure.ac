#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([src/iofwd/iofwd.c])
AC_CONFIG_HEADER([iofwd_config.h])

AC_LANG([C++])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP

AC_REQUIRE_CPP

# library locations
BMI_INCLUDE=
BMI_LIBS=
AC_ARG_WITH([bmi],[AS_HELP_STRING([--with-bmi], 
                        [location of BMI library])],
                   [bmidir=$withval])

if test "x$bmidir" != "x" ; then
    CPPFLAGS="$CPPFLAGS -I${bmidir}/include"
    LDFLAGS="$LDFLAGS -L${bmidir}/lib"
fi



# Checks for libraries.
AC_CHECK_LIB([bmi], [BMI_initialize])
AC_CHECK_LIB([pthread], [pthread_exit])


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([bmi.h], [], [AC_MSG_ERROR([Could not find bmi header!
                  Maybe you need to use --with-bmi=location?])])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT32_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])


# for fuse
AC_SYS_LARGEFILE 
PKG_CHECK_MODULES(FUSE, [fuse],,exit)
CFLAGS="$CFLAGS $FUSE_CFLAGS"
CPPFLAGS="$CPPFLAGS $FUSE_CFLAGS"
LDFLAGS="$LDFLAGS $FUSE_LIBS"

# check for hints parameter in BMI functions
AC_MSG_CHECKING([if BMI uses hints])
AC_COMPILE_IFELSE(
                  [#include <bmi.h>
                   int main(int argc, char**args)
                   { BMI_post_send(0,0,0,0,BMI_PRE_ALLOC,0,0,0,0); return 0; }],
                  [AC_DEFINE([BMI_HINTS], 1, [If BMI supports hints])
                   AC_MSG_RESULT([yes])], [AC_MSG_RESULT([no])])





# find out some basic paths

AC_CONFIG_FILES([
                 Makefile
                 sandbox/module.mk
                 src/backend/module.mk
                 src/frontend/module.mk
                 src/iofwd/module.mk
                 src/mgmt/module.mk
                 src/iofwdutil/module.mk
                 src/metacache/module.mk
                 src/cache/module.mk
                 src/fuse-zoid/module.mk
                 src/zoidfs-posix/module.mk
                 src/zoidfs-bmi/module.mk
                 test/module.mk
                 ])

AC_OUTPUT

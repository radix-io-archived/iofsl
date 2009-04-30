#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([src/iofwd/iofwd.cpp])
AC_CONFIG_HEADER([iofwd_config.h])


# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP

AC_REQUIRE_CPP

AC_LANG([C++])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h execinfo.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T


# Checks for library functions.
AC_CHECK_FUNCS([memset])
AC_CHECK_LIB([rt],[clock_getres],,[AC_MSG_ERROR([Could not find clock_getres!])])


# BMI
AC_LANG_PUSH([C])
BMI_INCLUDE=
BMI_LIBS=
AC_ARG_WITH([bmi],[AS_HELP_STRING([--with-bmi], 
                        [location of BMI library])],
                   [bmidir=$withval])
if test "x$bmidir" != "x" ; then
    CPPFLAGS="$CPPFLAGS -I${bmidir}/include"
    LDFLAGS="$LDFLAGS -L${bmidir}/lib"
fi
AC_CHECK_LIB([bmi], [BMI_initialize],,
             [AC_MSG_ERROR([Could not find usable bmi library!])])
AC_LANG_POP

# check boost version

MINE_FINDLIBEXT

dnl AC_CANONICAL_SYSTEM
dnl AC_CANONICAL_HOST
dnl test -z "$host" && AC_MSG_ERROR([host variable is empty??])
dnl host_os=`echo "$host" | sed 's/^\([^-]*\)-\([^-]*\)-\(.*\)$/\3/'`
dnl libext=a
dnl case "$host_os" in
dnl   cygwin* | mingw* | pw32* | cegcc*)
dnl     libext=lib
dnl     ;;
dnl esac


BOOST_REQUIRE([1.34.1])
BOOST_THREADS
BOOST_PROGRAM_OPTIONS
AX_MAKE_VAR([BOOST_PROGRAM_OPTIONS_LIBS])
AX_MAKE_VAR([BOOST_PROGRAM_OPTIONS_LDFLAGS])
AX_MAKE_VAR([BOOST_THREAD_LIBS])
AX_MAKE_VAR([BOOST_THREAD_LDFLAGS])

#m4_pattern_allow(BOOST_THREAD_LDFLAGS)
CPPFLAGS="${CPPFLAGS} ${BOOST_CPPFLAGS}"
#LIBS="${LIBS} ${BOOST_THREAD_LIBS}"

# for fuse
dnl AC_SYS_LARGEFILE 
dnl PKG_CHECK_MODULES(FUSE, [fuse],,exit)
dnl CFLAGS="$CFLAGS $FUSE_CFLAGS"
dnlCPPFLAGS="$CPPFLAGS $FUSE_CFLAGS"
dnl LDFLAGS="$LDFLAGS $FUSE_LIBS"

AX_OUTPUT_MAKE_VARS

AC_CONFIG_FILES([
                 Makefile
                 src/spatial/module.mk
                 src/sm/module.mk
                 src/iofwdevent/module.mk
                 src/iofwd/module.mk
                 src/iofwdutil/module.mk
                 src/metacache/module.mk
                 src/client/module.mk
                 src/cache/module.mk
                 src/taskmon/module.mk
                 test/module.mk
                 sandbox/module.mk
                 ])

AC_OUTPUT

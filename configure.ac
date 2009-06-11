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


dnl For fuse
dnl Add this before checking the size of size_t / ssize_t
AX_ADD_DEFINE([_FILE_OFFSET_BITS=64])


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
# check that is is recent enough to have BMI_TCP_CHECK_UNEXPECTED
AC_CHECK_HEADER_DEFINE([<bmi.h>],[BMI_TCP_CHECK_UNEXPECTED],[],
        AC_MSG_ERROR([BMI does not have BMI_TCP_CHECK_UNEXPECTED]))
AC_LANG_POP

# Search for PVFS
AX_LIB_PVFS2
if test "A$PVFS2_OK" == "A1" ; 
then
        DISPATCH_PVFS=1
        AX_MAKE_VAR([DISPATCH_PVFS])
fi

AC_ARG_WITH(db,
           [AC_HELP_STRING([--with-db=<dir>],
                  [Location of installed DB package (default= /usr)])],
           [AX_LIB_DB(${withval})],
           [AX_LIB_DB("")])
dnl AM_CONDITIONAL(HAVE_DB, test x${BDB_OK} == x1)
AX_MAKE_VAR([HAVE_DB])
AC_SUBST([BDB_CFLAGS])
AC_SUBST([BDB_CPPFLAGS])
AC_SUBST([BDB_LDFLAGS])

dnl # for fuse
use_fuse=1
AX_FUSE([AC_DEFINE([HAVE_FUSE],[1],[Is fuse available])],
  [use_fuse=0])
dnl AM_CONDITIONAL([BUILD_FUSE], [test "x${use_fuse}" == "x1"])
AX_MAKE_VAR([FUSE_LIBS])
AX_MAKE_VAR([FUSE_LDFLAGS])
AX_MAKE_VAR([FUSE_CPPFLAGS])
AX_MAKE_VAR([FUSE_CFLAGS])

dnl # mysql
AX_LIB_MYSQL([MYSQL_OK=1])
dnl AM_CONDITIONAL(HAVE_MYSQL,test x${MYSQL_OK} == x1)
AC_SUBST([MYSQL_CFLAGS])
AC_SUBST([MYSQL_CPPFLAGS])
AC_SUBST([MYSQL_LDFLAGS_R])

DISPATCH_POSIX=0
if test "x${BDB_OK}${MYSQL_OK}"; then
   AC_DEFINE([HAVE_DISPATCHER_POSIX],[1],[Is posix dispatcher available])
   DISPATCH_POSIX=1
fi
AX_MAKE_VAR([DISPATCH_POSIX])

dnl
dnl Control behaviour of the zoidfs posix mapping
dnl
AC_ARG_ENABLE([stale],
      [AC_HELP_STRING([--enable-stale],
                [Allow the POSIX layer to return ESTALE])],
      [case "${enableval}" in 
         yes) use_stale=true;;
         no)  use_stale=false;;
         *)  AC_MSG_ERROR([bad value ${enableval} for --enable-stale]) ;;
     esac],
     [use_stale=false])
if test x${use_stale} == xtrue ; then
   AC_DEFINE([USE_ESTALE],[1],[If the API can use ESTALE])
fi




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
BOOST_REGEX
BOOST_DATE_TIME
AX_MAKE_VAR([BOOST_PROGRAM_OPTIONS_LIBS])
AX_MAKE_VAR([BOOST_PROGRAM_OPTIONS_LDFLAGS])
AX_MAKE_VAR([BOOST_THREAD_LIBS])
AX_MAKE_VAR([BOOST_THREAD_LDFLAGS])
AX_MAKE_VAR([BOOST_REGEX_LIBS])
AX_MAKE_VAR([BOOST_REGEX_LDFLAGS])
AX_MAKE_VAR([BOOST_DATE_TIME_LIBS])
AX_MAKE_VAR([BOOST_DATE_TIME_LDFLAGS])

#m4_pattern_allow(BOOST_THREAD_LDFLAGS)
CPPFLAGS="${CPPFLAGS} ${BOOST_CPPFLAGS}"
#LIBS="${LIBS} ${BOOST_THREAD_LIBS}"

# for fuse
dnl AC_SYS_LARGEFILE 
dnl PKG_CHECK_MODULES(FUSE, [fuse],,exit)
dnl CFLAGS="$CFLAGS $FUSE_CFLAGS"
dnlCPPFLAGS="$CPPFLAGS $FUSE_CFLAGS"
dnl LDFLAGS="$LDFLAGS $FUSE_LIBS"

dnl 
dnl  OPTIONS
dnl
AC_ARG_ENABLE([valgrind], AS_HELP_STRING([--enable-valgrind], 
                                         [Try harder to be valgrind-clean]),
                                   [AX_ADD_DEFINE([VALGRIND_SAFE])])

AX_OUTPUT_MAKE_VARS

AC_CONFIG_FILES([
                 Makefile
                 src/spatial/module.mk
                 src/sm/module.mk
                 src/iofwdevent/module.mk
                 src/iofwd/module.mk
                 src/iofwdutil/module.mk
                 src/metacache/module.mk
                 src/cache/module.mk
                 src/taskmon/module.mk
                 src/zoidfs/module.mk
                 src/fuse/module.mk
                 src/client/module.mk
                 src/c-util/module.mk
                 test/module.mk
                 sandbox/module.mk
                 ])

AC_OUTPUT

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([iofwd], [0.1], [Submit trouble tickets at http://trac.mcs.anl.gov/projects/iofsl/newticket], [], [http://trac.mcs.anl.gov/projects/iofsl/wiki])

AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([foreign subdir-objects])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/iofwd/iofwd.cpp])
AC_CONFIG_HEADER([iofwd_config.h])



# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_RANLIB

AC_REQUIRE_CPP

ENABLE_MPI=
ORIG_CC=
ORIG_CPP=
ORIG_CXX=
AC_ARG_WITH([mpi],[AS_HELP_STRING([--with-mpi@<:@=DIR@:>@], 
                        [location of MPI library])],
                   [mpidir=$withval])

# search for mpi compilers and overide the normal compilers
# if mpi compilers are found
#
# if the mpi flag is specifed but the mpi compilers are not found
# ... abort
if test "x${mpidir}" != "x" ; then
    AC_CHECK_PROG([MPICC],[mpicc],[${mpidir}/bin/mpicc],[0], [${mpidir}/bin])
    if test "x$MPICC" != "x0" ; then
        ORIG_CC="${CC}"
        CC="${mpidir}/bin/mpicc"
    else
        AC_MSG_ERROR([Could not locate mpicc!])
    fi
    AC_CHECK_PROG([MPICPP],[mpic++],[${mpidir}/bin/mpic++],[0], [${mpidir}/bin])
    if test "x$MPICPP" != "x0" ; then
        ORIG_CPP="${CPP}"
        CPP="${mpidir}/bin/mpicpp"
    else
        AC_MSG_ERROR([Could not locate mpic++!])
    fi
    AC_CHECK_PROG([MPICXX],[mpicxx],[${mpidir}/bin/mpicxx],[0], [${mpidir}/bin])
    if test "x$MPICXX" != "x0" ; then
        ORIG_CXX="${CXX}"
        CXX="${mpidir}/bin/mpicxx"
    else
        AC_MSG_ERROR([Could not locate mpicxx!])
    fi
    ENABLE_MPI=1
fi
AX_MAKE_VAR([ENABLE_MPI])

AX_PROG_BISON([],[AC_MSG_ERROR([No BISON can be found!])])
AX_PROG_FLEX([],[AC_MSG_ERROR([No FLEX can be found!])])
AC_SUBST([BISON])
AC_SUBST([FLEX])

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


# Check for CUNIT
cunitdir=
AC_ARG_WITH([cunit],[AS_HELP_STRING([--with-cunit@<:@=DIR@:>@], 
                        [location of cunit library])],
                   [cunitdir=$withval])
ENABLE_CUNIT=

if test "A$cunitdir" != "Ano" ; 
then
  #
  # backup the original flags
  #
  ORIG_LDFLAGS="${LDFLAGS}"
  ORIG_CPPFLAGS="${CPPFLAGS}"
  ORIG_LIBS="${LIBS}"
  ORIG_CFLAGS="${CFLAGS}"
  if test ! -z "$cunitdir" ; then 
    LDFLAGS="-L${cunitdir}/lib"
    CPPFLAGS="-I${cunitdir}/include"
    CFLAGS="-I${cunitdir}/include"
    LIBS="-lcunit"
  fi
  ENABLE_CUNIT=1
  AC_CHECK_HEADER([CUnit/Basic.h], [], [ENABLE_CUNIT=;])
  AC_CHECK_LIB([cunit],[CU_basic_run_tests],,[ENABLE_CUNIT=;])
  if test -z "$ENABLE_CUNIT" ; then
     AC_MSG_WARN([Could not find usable cunit library -- disabling unit tests])
  else
    AC_SUBST([CUNIT_LDFLAGS],["${LDFLAGS}"])
    AC_SUBST([CUNIT_CFLAGS],["${CFLAGS}"])
    AC_SUBST([CUNIT_CPPFLAGS],["${CPPFLAGS}"])
    AC_SUBST([CUNIT_LIBS],["${LIBS}"])
  fi
  #
  # reset to the original flags
  #
  LDFLAGS="${ORIG_LDFLAGS}"
  CPPFLAGS="${ORIG_CPPFLAGS}"
  LIBS="${ORIG_LIBS}"
  CFLAGS="${ORIG_CFLAGS}"
fi
AX_MAKE_VAR([ENABLE_CUNIT])
AX_MAKE_VAR([cunitdir])

# BMI
AC_LANG_PUSH([C])
BMI_INCLUDE=
BMI_LIBS=
AC_ARG_WITH([bmi],[AS_HELP_STRING([--with-bmi@<:@=DIR@:>@], 
                        [location of BMI library])],
                   [bmidir=$withval])
if test "x$bmidir" != "x" ; then
    CPPFLAGS="$CPPFLAGS -I${bmidir}/include"
    CXXCPPFLAGS="$CXXCPPFLAGS -I${bmidir}/include"
    LDFLAGS="$LDFLAGS -L${bmidir}/lib"
fi
AC_CHECK_LIB([bmi], [BMI_initialize],,
             [AC_MSG_ERROR([Could not find usable bmi library!])])
# check that is is recent enough to have BMI_TCP_CHECK_UNEXPECTED
AC_CHECK_HEADER_DEFINE([<bmi.h>],[BMI_TCP_CHECK_UNEXPECTED],[],
        AC_MSG_ERROR([BMI does not have BMI_TCP_CHECK_UNEXPECTED]))
AC_LANG_POP

#
# dispatcher flags
#  collect the cflags, ldflags, and libs flags for each dispatcher
#
ZFS_DISPATCHER_CFLAGS=
ZFS_DISPATCHER_CPPFLAGS=
ZFS_DISPATCHER_LDFLAGS=
ZFS_DISPATCHER_LIBS=

dnl
dnl Search for PVFS 
dnl 
AX_LIB_PVFS2
if test "A$PVFS2_OK" == "A1" ; 
then
        AC_DEFINE([HAVE_DISPATCHER_PVFS2],[1],[If the PVFS2 dispatcher module
                   needs to be built])
fi
AM_CONDITIONAL([DISPATCH_PVFS],[test "A${PVFS2_OK}" == "A1"])

dnl
dnl
dnl # for fuse
dnl Control building of FUSE client
dnl 
use_fuse=0
AC_ARG_ENABLE([fuse],
              [AC_HELP_STRING([--enable-fuse],
                              [Enable FUSE ZoidFS client])],
              [case "${enableval}" in
                  yes) use_fuse=1;;
                  no) use_fuse=0;;
                  *) AC_MSG_ERROR([bad value ${enableval} for
                                   --enable-fuse]) ;;
               esac],
               [use_fuse=0])

AM_CONDITIONAL([BUILD_FUSE_CLIENT], [test ${use_fuse} == 1])
AC_SUBST([FUSE_LIBS])
AC_SUBST([FUSE_LDFLAGS])
AC_SUBST([FUSE_CPPFLAGS])
AC_SUBST([FUSE_CFLAGS])

#
#  Dispatcher POSIX driver control
#
AC_ARG_ENABLE([posix],
              [AC_HELP_STRING([--enable-posix],
                              [Enable new POSIX zoidfs driver])],
              [case "${enableval}" in 
                  yes) use_posix=1;;
                  no) use_posix=0;;
                  *) AC_MSG_ERROR([bad value ${enableval} for
                                   --enable-posix]) ;;
               esac],
               [use_posix=1])


DISPATCH_POSIX=0
AC_MSG_CHECKING([If the POSIX dispatcher driver needs to be built])
if test "${use_posix}" == "1"; then
   AC_DEFINE([HAVE_DISPATCHER_POSIX],[1],[Build POSIX zoidfs dispatcher driver])
   DISPATCH_POSIX=1
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([DISPATCH_POSIX],[test ${use_posix} == 1])

#
# libsysio dispatcher build env setup
# Determine is sysio is available and where it is located if it is in a non-standard location
#
use_libsysio=false
AC_ARG_WITH(libsysio,
[  --with-libsysio@<:@=DIR@:>@  Location of installed libsysio package],
       if test "x$withval" != "x" ; then
              AC_SUBST([SYSIO_LDFLAGS],["-L${withval}/lib"])
              AC_SUBST([SYSIO_CFLAGS],["-I${withval}/include -I${withval}/misc -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64"])
              AC_SUBST([SYSIO_CPPFLAGS],["-I${withval}/include -I${withval}/misc -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64"])
              AC_SUBST([SYSIO_LIBS],["-lsysio"])
              AC_DEFINE([HAVE_DISPATCHER_LIBSYSIO],[1],[If libsysio is supported on target build platform])
              DISPATCH_LIBSYSIO=1
              dnl DISPATCH_POSIX=0
              dnl @TODO: disable posix before calling AM_CONDITIONAL(DISPATCH_POSIX, ...)
              use_libsysio=true
	fi)

AM_CONDITIONAL([DISPATCH_LIBSYSIO],[test x${use_libsysio} == xtrue])

if test x${use_libsysio} == xtrue ; then
    #
    # Check that the sysio headers are available 
    #
    STCPPFLAGS="${CPPFLAGS}"
    STCFLAGS="${CFLAGS}"
    STLDFLAGS="${LDFLAGS}"
    STLIBS="${LIBS}"

    CPPFLAGS="${CPPFLAGS} ${SYSIO_CPPFLAGS} -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64"
    CFLAGS="${CFLAGS} ${SYSIO_CFLAGS} -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64"
    LDFLAGS="${LDFLAGS} ${SYSIO_LDFLAGS}"
    LIBS="${LIBS}"

    AC_CHECK_HEADER([fhi.h], [], [AC_MSG_ERROR([Could not find the libsysio header files!])], 
    [
    #include <stdlib.h>
    #include <string.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/uio.h>
    #include <getopt.h>
    #include <sys/statvfs.h>
    #include <dirent.h>
    #include <errno.h>
    #include <pthread.h>

    #if defined(SYSIO_LABEL_NAMES)
    #include "sysio.h"
    #endif

    #include "xtio.h"
    ])

    AC_CHECK_LIB([sysio], [_sysio_shutdown], [], [AC_MSG_ERROR([Could not find libsysio library!])], [])
    AC_CHECK_FUNC([_sysio_native_init], [], [AC_MSG_ERROR([libsysio was built without support for the libsysio native driver!])])
    AC_CHECK_FUNC([_zfs_sysio_fhi_lookup], [], [AC_MSG_ERROR([libsysio was built without the required zoidfs symbols! Check that libsysio was configured with '--with-file-handle-interface=yes --with-alternate-symbols="_zfs_sysio_"' for proper zoidfs support.])])
    AC_CHECK_FUNC([_zfs_sysio_statvfs], [], [AC_MSG_ERROR([libsysio was built without statvfs support!])])

    #
    #
    # Need these sizes for determining how the
    # sysio handles are packed into the zoidfs handle
    #
    AC_CHECK_SIZEOF([struct file_handle_info_export *])
    AC_CHECK_SIZEOF([uint8_t])
    AC_CHECK_SIZEOF([size_t])

    #
    # Add sysio flags to the dispatcher set of flags
    #
    ZFS_DISPATCHER_CFLAGS="${SYSIO_CFLAGS} -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64 ${ZFS_DISPATCHER_CFLAGS}"
    ZFS_DISPATCHER_CPPFLAGS="${SYSIO_CPPFLAGS} -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64 ${ZFS_DISPATCHER_CPPFLAGS}"
    ZFS_DISPATCHER_LDFLAGS="${SYSIO_LDFLAGS} ${ZFS_DISPATCHER_LDFLAGS}"
    ZFS_DISPATCHER_LIBS="-lsysio ${ZFS_DISPATCHER_LIBS}"

    #
    # Reset the flags
    #
    CPPFLAGS="${STCPPFLAGS}"
    CFLAGS="${STCFLAGS}"
    LDFLAGS="${STLDFLAGS}"
    LIBS="${STLIBS}"
fi

#
# save the ZFS_DISPATCHER flags
#
AC_SUBST([ZFS_DISPATCHER_CFLAGS])
AC_SUBST([ZFS_DISPATCHER_CPPFLAGS])
AC_SUBST([ZFS_DISPATCHER_LDFLAGS])
AC_SUBST([ZFS_DISPATCHER_LIBS])

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
AC_SUBST([BOOST_PROGRAM_OPTIONS_LIBS])
AC_SUBST([BOOST_PROGRAM_OPTIONS_LDFLAGS])
AC_SUBST([BOOST_THREAD_LIBS])
AC_SUBST([BOOST_THREAD_LDFLAGS])
AC_SUBST([BOOST_REGEX_LIBS])
AC_SUBST([BOOST_REGEX_LDFLAGS])
AC_SUBST([BOOST_DATE_TIME_LIBS])
AC_SUBST([BOOST_DATE_TIME_LDFLAGS])


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
                                 dnl  [AX_ADD_DEFINE([VALGRIND_SAFE])])
    [AC_DEFINE([VALGRIND_SAFE],[1],[Clear memory to avoid valgrind warnings])
     VALGRIND_SAFE=1],
     [VALGRIND_SAFE=0])
AC_MSG_CHECKING([If we try harder to be valgrind clean])
if test ${VALGRIND_SAFE} == "1"; then
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT([no])
fi

AX_OUTPUT_MAKE_VARS

AC_CONFIG_FILES([
                 Makefile
                 ])

AC_OUTPUT

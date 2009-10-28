#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([iofwd],[0.1],[Submit trouble tickets at http://trac.mcs.anl.gov/projects/iofsl/newticket],[],[http://trac.mcs.anl.gov/projects/iofsl/wiki])

AC_CANONICAL_TARGET
AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

# removed color-tests... no supported by 1.10.2
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/iofwd/iofwd.cpp])
AC_CONFIG_HEADERS([iofwd_config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
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
AM_CONDITIONAL([HAVE_MPI],[test "x${ENABLE_MPI}" == "x1"])

dnl
dnl Conditional client / server build args
dnl
AC_MSG_CHECKING(if IOFWD server should be built)
AC_ARG_ENABLE([iofwd-server],
              [AS_HELP_STRING([--disable-iofwd-server], [Do not build the IOFWD server and supporting libraries.])])
if test "x${enable_iofwd-client}" == "xyes" ; then
   AC_MSG_RESULT(yes) 
else 
    if test "x${enable_iofwd-client}" == "xno" ; then
        AC_MSG_RESULT(no)
    else
        AC_MSG_RESULT(auto)
    fi 
fi
AM_CONDITIONAL([BUILD_IOFWD_SERVER], [test "x${enableval}" != "xno"])

AC_MSG_CHECKING(if IOFWD client should be built)
AC_ARG_ENABLE([iofwd-client],
              [AS_HELP_STRING([--disable-iofwd-client], [Do not build the IOFWD client and supporting libraries.])])
if test "x${enable_iofwd-client}" == "xyes" ; then
   AC_MSG_RESULT(yes) 
else
    if test "x${enable_iofwd-client}" == "xno" ; then
        AC_MSG_RESULT(no)
    else
        AC_MSG_RESULT(auto)
    fi 
fi

AM_CONDITIONAL([BUILD_IOFWD_CLIENT], [test "x${enableval}" != "xno"])

dnl
dnl Flex/bison
dnl
AX_PROG_BISON([],[AC_MSG_ERROR([No BISON can be found!])])
AX_PROG_FLEX([],[AC_MSG_ERROR([No FLEX can be found!])])
AC_SUBST([BISON])
AC_SUBST([FLEX])

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
AM_CONDITIONAL([HAVE_CUNIT],[test "x${ENABLE_CUNIT}" == "x1"])

# ===========================================================================
# ====== BMI ================================================================
# ===========================================================================

BMI_CPPFLAGS=
BMI_LIBS=
BMI_LDFLAGS=

AC_ARG_WITH([bmi],[AS_HELP_STRING([--with-bmi@<:@=DIR@:>@], 
                        [location of BMI library])],
                   [bmidir=$withval])
   

if test "x$bmidir" != "x" ; then
   BMI_CPPFLAGS="-I${bmidir}/include"
   BMI_LIBS="-lbmi"
   BMI_LDFLAGS="-L${bmidir}/lib"
fi
    
OLDCPPFLAGS="$CPPFLAGS"
OLDLDFLAGS="$LDFLAGS"
CPPFLAGS="$CPPFLAGS $BMI_CPPFLAGS"
LDFLAGS="$LDFLAGS $BMI_LDFLAGS"

AC_LANG_PUSH([C])
AC_CHECK_LIB([bmi], [BMI_initialize],[BMI_LIBS="-lbmi"],
             [AC_MSG_ERROR([Could not find usable bmi library!])])
# check that is is recent enough to have BMI_TCP_CHECK_UNEXPECTED
AC_CHECK_HEADER_DEFINE([<bmi.h>],[BMI_TCP_CHECK_UNEXPECTED],[],
        AC_MSG_ERROR([BMI does not have BMI_TCP_CHECK_UNEXPECTED]))
AC_LANG_POP

CPPFLAGS="$OLDCPPFLAGS"
LDFLAGS="$OLDLDFLAGS"

AC_SUBST([BMI_CPPFLAGS])
AC_SUBST([BMI_LDFLAGS])
AC_SUBST([BMI_LIBS])

# Check for differences in size_t and bmi_size_t
AC_CHECK_SIZEOF([size_t])
AC_CHECK_SIZEOF([int64_t]) # bmi_size_t equivalent

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

dnl =========================================================================
dnl      FUSE client
dnl =========================================================================

dnl Control building of FUSE client

AC_MSG_CHECKING(if FUSE client support enabled)
AC_ARG_ENABLE([fuse], [AS_HELP_STRING([--disable-fuse],[Disable FUSE ZoidFS client])])
case "${enable_fuse}" in
    yes) use_fuse=1;;
    no) use_fuse=0;;
    '') use_fuse=a;;
    *) AC_MSG_ERROR([bad value ${enableval} for --disable-fuse])
esac
if test "x${use_fuse}" == "x0" ; then
        AC_MSG_RESULT(no)
else
    if test "x${use_fuse}" == "x1" ; then
        AC_MSG_RESULT(yes)
    else
        AC_MSG_RESULT(auto)
    fi
fi

# unless the user disabled fuse support, try to locate fuse and use it.
if test "x${use_fuse}" != "x0" ; then
   AX_FUSE([found_fuse=1], [found_fuse=0])
else
   found_fuse=0
   AC_MSG_NOTICE([FUSE support disabled by the user (--disable-fuse)])
fi

if test "x${use_fuse}" == "x1" -a "x${found_fuse}" != "x1" ; then
   AC_MSG_ERROR([FUSE support requested but could not locate FUSE!])
fi

AM_CONDITIONAL([HAVE_FUSE], [test ${found_fuse} == 1])
AC_SUBST([FUSE_LIBS])
AC_SUBST([FUSE_LDFLAGS])
AC_SUBST([FUSE_CPPFLAGS])
AC_SUBST([FUSE_CFLAGS])


dnl =========================================================================
dnl     Dispatcher POSIX driver control
dnl =========================================================================

AC_ARG_ENABLE([posix],
              [AS_HELP_STRING([--enable-posix],[Enable new POSIX zoidfs driver])],
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

dnl =========================================================================
dnl    SYSIO
dnl =========================================================================

# libsysio dispatcher build env setup
# Determine is sysio is available and where it is located if it is in a
# non-standard location

use_libsysio=false
AC_ARG_WITH(libsysio,
[  --with-libsysio@<:@=DIR@:>@  Location of installed libsysio package],
       if test "x$withval" != "x" ; then
          SYSIO_LDFLAGS="-L${withval}/lib"
          SYSIO_CFLAGS=
          SYSIO_CPPFLAGS="-I${withval}/include -I${withval}/misc"
          SYSIO_LIBS="-lsysio"
          AC_SUBST([SYSIO_LDFLAGS])
          AC_SUBST([SYSIO_CFLAGS])
          AC_SUBST([SYSIO_CPPFLAGS])
          AC_SUBST([SYSIO_LIBS])
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

    SYSIO_CPPFLAGS="${SYSIO_CPPFLAGS} -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64"

    CPPFLAGS="${CPPFLAGS} ${SYSIO_CPPFLAGS}"
    CFLAGS="${CFLAGS} ${SYSIO_CFLAGS}"
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
ZFS_DISPATCHER_CFLAGS="${SYSIO_CFLAGS} ${PVFS2_CFLAGS}"
ZFS_DISPATCHER_CPPFLAGS="${SYSIO_CPPFLAGS} -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64 ${PVFS2_CPPFLAGS}"
ZFS_DISPATCHER_LDFLAGS="${SYSIO_LDFLAGS} ${PVFS2_LDFLAGS}"
ZFS_DISPATCHER_LIBS="${SYSIO_LIBS} ${PVFS2_LIBS} ${SYSIO_LIBS}"

AC_SUBST([ZFS_DISPATCHER_CFLAGS])
AC_SUBST([ZFS_DISPATCHER_CPPFLAGS])
AC_SUBST([ZFS_DISPATCHER_LDFLAGS])
AC_SUBST([ZFS_DISPATCHER_LIBS])


dnl =========================================================================
dnl    BOOST
dnl =========================================================================

# boost m4 macros rely on libtool to set libext.
# We don't want libtool so we compute it ourselves.
AX_FINDLIBEXT

# check boost version
# macro defines BOOST_CPPFLAGS, BOOST_LDFLAGS 
# and BOOST_THREAD_LIB, BOOST_REGEX_LIB, ...
# is already subst
AX_BOOST_BASE([1.35])
AX_BOOST_THREAD
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_REGEX
AX_BOOST_DATE_TIME
AX_BOOST_UNIT_TEST_FRAMEWORK

AX_BOOST_SPIRIT_OLD

dnl =========================================================================
dnl  Valgrind
dnl =========================================================================

AC_ARG_ENABLE([valgrind], AS_HELP_STRING([--enable-valgrind], 
                                         [Try harder to be valgrind-clean]),
    [AC_DEFINE([VALGRIND_SAFE],[1],[Clear memory to avoid valgrind warnings])
     VALGRIND_SAFE=1],
     [VALGRIND_SAFE=0])
AC_MSG_CHECKING([If we try harder to be valgrind clean])
if test ${VALGRIND_SAFE} == "1"; then
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT([no])
fi



dnl =========================================================================
dnl =========================================================================
dnl =========================================================================

AC_CONFIG_FILES([
                 Makefile
                 ])

AC_OUTPUT

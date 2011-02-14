#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([iofwd],[0.1],[Submit trouble tickets at http://trac.mcs.anl.gov/projects/iofsl/newticket],[],[http://trac.mcs.anl.gov/projects/iofsl/wiki])

AC_CANONICAL_TARGET
AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

# removed color-tests... not supported by 1.10.2
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/iofwd/iofwd.cpp])
AC_CONFIG_HEADERS([iofwd_config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_RANLIB

dnl =========================================================================
dnl == Google's PerfTools ===================================================
dnl =========================================================================

AC_ARG_WITH(perftools,
    AS_HELP_STRING([--with-perftools=dir],
                   [Location of google perftools installation]),
    PERFTOOL_DIR="$withval",PERFTOOL_DIR="")

AC_MSG_WARN($PERFTOOL_DIR)

if test -n "$PERFTOOL_DIR" ;
then
   CPPFLAGS="$CPPFLAGS -I${PERFTOOL_DIR}/include"
   LDFLAGS="$LDFLAGS -L${PERFTOOL_DIR}/lib"
   IOFSL_CLIENT_CFLAGS_PKG="${IOFSL_CLIENT_CFLAGS_PKG} -I${PERFTOOL_DIR}/include"
   IOFSL_CLIENT_LIBS_PKG="${IOFSL_CLIENT_LIBS_PKG} -L${PERFTOOL_DIR}/lib -ltcmalloc"
fi

if test -n "$PERFTOOL_DIR" ;
then
use_perftools=1
else
use_perftools=
fi

AC_CHECK_HEADERS([google/tcmalloc.h],[],
                 [use_perftools=])

if test -n "$use_perftools"; then
AC_CHECK_LIB([tcmalloc],[tc_new],[],[use_perftools=])
fi

AM_CONDITIONAL([HAVE_PERFTOOLS],[test "x${use_perftools}" == "x1"])

AC_MSG_CHECKING([if Google Perftools are used])
if test "x${use_perftools}" == "x1" ; then
      AC_DEFINE([HAVE_PERFTOOLS], [1], [If google perftools are available])
      AC_MSG_RESULT([yes])
else
      AC_MSG_RESULT([no])
fi

dnl ==================================================================
dnl === MPI ==========================================================
dnl ==================================================================

AC_ARG_WITH([mpi],[AS_HELP_STRING([--with-mpi@<:@=DIR@:>@],
                        [location of MPI library])],
                   [],
                   [with_mpi=check])

ENABLE_MPI=
ORIG_CC=
ORIG_CPP=
ORIG_CXX=
AS_IF([test "x$with_mpi" != "xno"],
[
 if test "x$with_mpi" != "xcheck" -a "x$with_mpi" != "xyes"; then
    checkpath="${with_mpi}/bin"
 else
    checkpath="$PATH"
 fi
 AC_CHECK_PROGS(MPICC, [mpicc], [], [$checkpath])
 #
 # with mpich2-1.3, mpiCC invokes gcc... not g++
 #
 AC_CHECK_PROGS(MPICXX, [mpic++ mpicxx mpiCC], [], [$checkpath])
 if test "x$with_mpi" != "xcheck"; then
    if test -z "${MPICXX}" ; then
    AC_MSG_ERROR("--with-mpi given but could not find mpic++!")
    fi
    if test -z "${MPICC}" ; then
    AC_MSG_ERROR("--with-mpi given but could not find mpicc!")
    fi
 fi
 ])

if test "${MPICXX}x" != "x" ; then
   if test "${MPICC}x" == "x"; then
      AC_MSG_ERROR("Found MPI C++ compiler but not MPI C compiler??")
   fi

   #
   # make sure mpi compiler path is a full path
   #
   ORIG_CXX="${CXX}"
   if test "x$with_mpi" != "xcheck" -a "x$with_mpi" != "xyes"; then
        CXX="${checkpath}/${MPICXX}"
   else
        CXX="${MPICXX}"
   fi

   ORIG_CC="${CC}"
   if test "x$with_mpi" != "xcheck" -a "x$with_mpi" != "xyes"; then
        CC="${checkpath}/${MPICC}"
   else
        CC="${MPICC}"
   fi
   ENABLE_MPI=1

fi

AC_MSG_CHECKING([if MPI support will be enabled])
if test "x$ENABLE_MPI" == "x1"; then
AC_MSG_RESULT([yes])
else
AC_MSG_RESULT([no])
fi

AM_CONDITIONAL([HAVE_MPI],[test "x${ENABLE_MPI}" == "x1"])
AC_DEFINE([HAVE_MPI],[test "x${ENABLE_MPI}" == "x1"],[Build with MPI support])

dnl ========================================================
dnl Now that we have a compiler, try to use preprocessor
dnl ========================================================
AC_REQUIRE_CPP
AC_PROG_CXXCPP

dnl =========================================================================
dnl ===== pkg-config support ================================================
dnl =========================================================================

dnl There are 2 pkg-config files: zoidfs and zoidfs-local
dnl Using zoidfs results in the forwarding version, zoidfs-local links
dnl directly with the dispatcher.

dnl
dnl CFLAGS is CPPFLAGS + CXXFLAGS + CFLAGS
dnl LIBS = LDFLAGS + LIBS
dnl STATIC_LIBS = _additional_ libraries needed when linking static
dnl

ZOIDFS_CFLAGS_PKG=
ZOIDFS_LOCAL_CFLAGS_PKG=

ZOIDFS_LIBS_PKG=
ZOIDFS_STATIC_LIBS_PKG=

ZOIDFS_LOCAL_LIBS_PKG=
ZOIDFS_LOCAL_STATIC_LIBS_PKG=

#
# IOFSL Client flags for pkg-config
#
IOFSL_CLIENT_CFLAGS_PKG=
IOFSL_CLIENT_LIBS_PKG=
IOFSL_CLIENT_STATIC_LIBS_PKG=

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
AX_PROG_BISON([],[AC_MSG_WARN([No BISON can be found!])])
AX_PROG_FLEX([],[AC_MSG_WARN([No FLEX can be found!])])
AC_SUBST([BISON])
AC_SUBST([FLEX])

AC_LANG([C++])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h execinfo.h pthread.h cxxabi.h])


# note that we should be using the C++ boost inttypes header instead.
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
AC_CHECK_LIB([pthread],[pthread_create],,[AC_MSG_ERROR([Could not find pthread_create!])])
IOFSL_CLIENT_LIBS_PKG="${IOFSL_CLIENT_LIBS_PKG} -lrt -lpthread"

dnl =========================================================================
dnl = Check if sig_atomic_t is working for C++                              =
dnl =========================================================================
AC_LANG_PUSH([C++])
AC_MSG_CHECKING(for sig_atomic_t in csignal)
AC_EGREP_HEADER(sig_atomic_t,csignal,
   [
     ac_cv_type_sig_atomic_t=yes;
     AC_EGREP_HEADER(volatile.*sig_atomic_t,
                     signal.h,
                     [
                         is_sig_atomic_t_volatile=yes;
                         AC_MSG_RESULT([yes, volatile])
                     ],
                     [
                         is_sig_atomic_t_volatile=no;
                         AC_MSG_RESULT([yes, non volatile])
                     ])
   ],
   [
     AC_MSG_RESULT(no)
     AC_CHECK_TYPES(sig_atomic_t,
                    [],
                    [AC_MSG_ERROR([No sig_atomic_t in csignal??])],
                    [csignal])
     is_sig_atomic_t_volatile=no
   ])
AC_LANG_POP

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

#
# Experimental optimizations flags
#

AC_ARG_ENABLE([ceo],
              [AS_HELP_STRING([--enable-ceo],[Enable experimental client optimizations])],
              [case "${enableval}" in
                  yes) use_ceo=1;;
                  no) use_ceo=0;;
                  *) AC_MSG_ERROR([bad value ${enableval} for
                                   --enable-ceo]) ;;
               esac],
               [use_ceo=1])

USE_CEO=0
CEO_CFLAGS=""
AC_MSG_CHECKING([If the client should be built with experimental optimizations])
if test "${use_ceo}" == "1"; then
   AC_DEFINE([USE_CEO],[1],[Build client with experimental optimizations])
   USE_CEO=1
   CEO_CFLAGS="-DZFS_USE_NB_BMI_COMM -DZFS_BMI_FASTMEMALLOC"
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([USE_CEO],[test ${use_ceo} == 1])

AC_ARG_ENABLE([seo],
              [AS_HELP_STRING([--enable-seo],[Enable experimental server optimizations])],
              [case "${enableval}" in
                  yes) use_seo=1;;
                  no) use_seo=0;;
                  *) AC_MSG_ERROR([bad value ${enableval} for
                                   --enable-seo]) ;;
               esac],
               [use_seo=1])

USE_SEO=0
SEO_CFLAGS=""
SEO_CXXFLAGS=""
AC_MSG_CHECKING([If the server should be built with experimental optimizations])
if test "${use_seo}" == "1"; then
   AC_DEFINE([USE_SEO],[1],[Build server with experimental optimizations])
   USE_SEO=1
   SEO_CFLAGS="-DZLOG_DISABLE_ALL -DITRS"
   SEO_CXXFLAGS="-DZLOG_DISABLE_ALL -DITRS"
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([USE_SEO],[test ${use_seo} == 1])

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
   IOFSL_CLIENT_LIBS_PKG="${IOFSL_CLIENT_LIBS_PKG} ${BMI_LDFLAGS} ${BMI_LIBS}"
   IOFSL_CLIENT_CFLAGS_PKG="${IOFSL_CLIENT_CFLAGS_PKG} ${BMI_CPPFLAGS}"
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

CPPFLAGS="$OLDCPPFLAGS $BMI_CPPFLAGS"
LDFLAGS="$OLDLDFLAGS $BMI_LDFLAGS$"

AC_SUBST([BMI_CPPFLAGS])
AC_SUBST([BMI_LDFLAGS])
AC_SUBST([BMI_LIBS])

#
# GridFTP
#
DISPATCH_GRIDFTP=0
GRIDFTP_CPPFLAGS=
GRIDFTP_CFLAGS=
GRIDFTP_LIBS=
GRIDFTP_LDFLAGS=

AC_ARG_WITH([gridftp],[AS_HELP_STRING([--with-gridftp@<:@=DIR@:>@],
                        [location of GridFTP library])],
                   [gridftpdir=$withval])


use_gridftp=0
if test "x$gridftpdir" != "x" ; then
    GRIDFTP_CPPFLAGS="-I${gridftpdir}/include/gcc64dbgpthr"
    GRIDFTP_LIBS="-lglobus_ftp_client_gcc64dbgpthr -lglobus_ftp_control_gcc64dbgpthr -lglobus_ftp_control_gcc64dbgpthr -lglobus_io_gcc64dbgpthr -lgssapi_error_gcc64dbgpthr -lglobus_gss_assist_gcc64dbgpthr -lglobus_gssapi_gsi_gcc64dbgpthr -lglobus_gsi_proxy_core_gcc64dbgpthr -lglobus_gsi_credential_gcc64dbgpthr -lglobus_gsi_callback_gcc64dbgpthr -lglobus_oldgaa_gcc64dbgpthr -lglobus_gsi_sysconfig_gcc64dbgpthr -lglobus_gsi_cert_utils_gcc64dbgpthr -lglobus_openssl_gcc64dbgpthr -lglobus_xio_gcc64dbgpthr -lglobus_openssl_error_gcc64dbgpthr -lglobus_callout_gcc64dbgpthr -lglobus_proxy_ssl_gcc64dbgpthr -lglobus_common_gcc64dbgpthr -lltdl_gcc64dbgpthr"
    GRIDFTP_LDFLAGS="-L${gridftpdir}/lib"

    OLDCPPFLAGS="$CPPFLAGS"
    OLDLDFLAGS="$LDFLAGS"
    OLDLIBS="$LIBS"
    CPPFLAGS="$CPPFLAGS $GRIDFTP_CPPFLAGS"
    LDFLAGS="$LDFLAGS $GRIDFTP_LDFLAGS"
    LIBS="$GRIDFTP_LIBS"

    AC_LANG_PUSH([C])
    AC_CHECK_LIB([globus_ftp_client_gcc64dbgpthr], [globus_ftp_client_handle_init],[],
             [AC_MSG_ERROR([Could not find usable gridftp library!])])
    AC_CHECK_LIB([globus_ftp_control_gcc64dbgpthr], [globus_ftp_control_handle_init],[],
             [AC_MSG_ERROR([Could not find usable gridftp library!])])
    AC_LANG_POP

    CPPFLAGS="$OLDCPPFLAGS"
    LDFLAGS="$OLDLDFLAGS"
    LIBS="$OLDLIBS"
    AC_DEFINE([USE_DISPATCHER_GRIDFTP],[1],[Build GridFTP zoidfs dispatcher driver])
    use_gridftp=1
fi

AC_SUBST([GRIDFTP_CPPFLAGS])
AC_SUBST([GRIDFTP_LDFLAGS])
AC_SUBST([GRIDFTP_LIBS])
AM_CONDITIONAL([DISPATCH_GRIDFTP],[test ${use_gridftp} == 1])

#
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
              [AS_HELP_STRING([--enable-posix],[Enable POSIX zoidfs driver])],
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
dnl     Dispatcher nofs driver control
dnl =========================================================================

AC_ARG_ENABLE([nofs],
              [AS_HELP_STRING([--enable-nofs],[Enable 'nofs' zoidfs driver])],
              [case "${enableval}" in
                  yes) use_nofs=1;;
                  no) use_nofs=0;;
                  *) AC_MSG_ERROR([bad value ${enableval} for
                                   --enable-nofs]) ;;
               esac],
               [use_nofs=0])

DISPATCH_NOFS=0
AC_MSG_CHECKING([If the nofs dispatcher driver needs to be built])
if test "${use_nofs}" == "1"; then
   AC_DEFINE([USE_DISPATCHER_NOFS],[1],[Build nofs zoidfs dispatcher driver])
   DISPATCH_NOFS=1
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([DISPATCH_NOFS],[test ${use_nofs} == 1])

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
ZFS_DISPATCHER_CFLAGS="${SYSIO_CFLAGS} ${PVFS2_CFLAGS} ${GRIDFTP_CFLAGS}"
ZFS_DISPATCHER_CPPFLAGS="${SYSIO_CPPFLAGS} -D_LARGEFILE64_SOURCE -DHAVE_POSIX_1003_READLINK -D__USE_LARGEFILE64 ${PVFS2_CPPFLAGS} ${GRIDFTP_CPPFLAGS}"
ZFS_DISPATCHER_LDFLAGS="${SYSIO_LDFLAGS} ${PVFS2_LDFLAGS} ${GRIDFTP_LDFLAGS}"
ZFS_DISPATCHER_LIBS="${SYSIO_LIBS} ${PVFS2_LIBS} ${SYSIO_LIBS} ${GRIDFTP_LIBS}"

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
AX_BOOST_BASE([1.36])
AX_BOOST_THREAD
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_REGEX
AX_BOOST_DATE_TIME
AX_BOOST_UNIT_TEST_FRAMEWORK

AC_SUBST([BOOST_PROGRAM_OPTIONS_LIB])
AC_SUBST([BOOST_DATE_TIME_LIB])

OLD_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
AX_BOOST_SPIRIT_OLD
CPPFLAGS="$OLD_CPPFLAGS"

dnl =========================================================================
dnl  gcc attribute support
dnl =========================================================================

AC_CACHE_CHECK([whether __attribute__ allowed],
pac_cv_gnu_attr_unused,[
AC_TRY_COMPILE([int foo(int) __attribute__ ((unused));],[int a;],
pac_cv_gnu_attr_unused=yes,pac_cv_gnu_attr_unused=no)])

dnl AC_CACHE_CHECK([whether __attribute__((format)) allowed],
dnl pac_cv_gnu_attr_format,[
dnl AC_TRY_COMPILE([int foo(char *,...) __attribute__ ((format(printf,1,2)));],[int a;],
dnl pac_cv_gnu_attr_format=yes,pac_cv_gnu_attr_format=no)])

if test "$pac_cv_gnu_attr_unused" = "yes" ; then
    AC_DEFINE(HAVE_GCC_ATTRIBUTE_UNUSED,[1],
              [Define if GNU __attribute__ unused is supported])
fi

dnl =========================================================================
dnl  Exception backtraces
dnl =========================================================================

dnl
dnl symbol name lookup requires -rdynamic
dnl
AC_ARG_ENABLE([exception-backtrace],
  AS_HELP_STRING([--enable-exception-backtrace],
     [Include backtrace in exceptions (debug option - slows down the server)]),
  [
     AC_DEFINE([EXCEPTION_BACKTRACE],[1],[Include backtrace in ZTHROW])
     LDFLAGS="$LDFLAGS -rdynamic"
  ]
  )

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
dnl ======== Atomic operations ==============================================
dnl =========================================================================
ATOMICS_CPPFLAGS=
ATOMICS_CFLAGS=
ATOMICS_LIBS=
ATOMICS_LDFLAGS=

AC_ARG_ENABLE([openpa],
            [AS_HELP_STRING([--disable-openpa],
               [Don't use OpenPA for atomic operations. (Default: try to use)])],
            [use_openpa=$withval],
            [use_openpa=check])
AC_ARG_ENABLE([glib],
            [AS_HELP_STRING([--disable-glib],
               [Don't use glib-2.0 for atomic operations. (Default: try to use)])],
            [use_glib=$withval],
            [use_glib=check])


OPENPA_OK=
GLIB_OK=

if test "A$use_openpa" != "Ano"; then
  AX_OPENPA
fi

if test "A${OPENPA_OK}" == "A1";
then
   AC_DEFINE([HAVE_OPENPA],[1],[If OpenPA can be used])
   AC_DEFINE([HAVE_ATOMICS], [1], [If fast atomics are available])
   ATOMICS_LDFLAGS="${OPENPA_LDFLAGS}"
   ATOMICS_CPPFLAGS="${OPENPA_CPPFLAGS} ${OPENPA_CFLAGS}"
   ATOMICS_LIBS="${OPENPA_LIBS}"
else
   if test "A$use_glib" != "Ano"; then
      AX_GLIB
   fi
   if test "A${GLIB_OK}" == "A1"
   then
      AC_DEFINE([HAVE_GLIB],[1],[If glib-2 can be used])
      AC_DEFINE([HAVE_ATOMICS], [1], [If fast atomics are available])
      ATOMICS_CPPFLAGS="${GLIB_CFLAGS} ${GLIB_CPPFLAGS}"
      ATOMICS_LDFLAGS="${GLIB_LDFLAGS}"
      ATOMICS_LIBS="${GLIB_LIBS}"
   else
      AC_MSG_WARN([
=====================================================================
== Could not find atomic operation support! Locks will be          ==
== used to emulate atomic operations. This is SLOW.                ==
== Please install OpenPA (http://trac.mcs.anl.gov/projects/openpa) ==
=====================================================================])
   fi
fi

if test "A$use_openpa" != "Ano" -a "A$use_openpa" != "Acheck" \
    -a "$OPENPA_OK" != "1" ; then
    AC_MSG_ERROR("--enable-openpa given but openpa could not be found!")
fi

if test "A$use_glib" != "Ano" -a "A$use_glib" != "Acheck" \
    -a "$GLIB_OK" != "1" ; then
    AC_MSG_ERROR("--enable-glib given but glib could not be found!")
fi

if test "$GLIB_OK" == "1"; then
   ATOMIC_LIBS="${ATOMIC_LIBS} ${GLIB_LIBS}"
   ATOMIC_CPPFLAGS="${ATOMIC_CPPFLAGS} ${GLIB_CPPFLAGS}"
   ATOMIC_CFLAGS="${ATOMIC_CFLAGS} ${GLIB_CFLAGS}"
   ATOMIC_LDFLAGS="${ATOMIC_LDFLAGS} ${GLIB_LDFLAGS}"
fi

AC_SUBST([ATOMICS_LIBS])
AC_SUBST([ATOMICS_CPPFLAGS])
AC_SUBST([ATOMICS_CFLAGS])
AC_SUBST([ATOMICS_LDFLAGS])

LIBS="$LIBS $ATOMIC_LIBS"
LDFLAGS="$LDFLAGS $ATOMIC_LDFLAGS"

AC_SUBST([ZOIDFS_CFLAGS_PKG])
AC_SUBST([ZOIDFS_LOCAL_CFLAGS_PKG])
AC_SUBST([ZOIDFS_LIBS_PKG])
AC_SUBST([ZOIDFS_LOCAL_LIBS_PKG])
AC_SUBST([ZOIDFS_STATIC_LIBS_PKG])
AC_SUBST([ZOIDFS_LOCAL_STATIC_LIBS_PKG])

dnl =========================================================================
dnl === Check if we have a BMI zoid that supports BMI_ZOID_POST_TIMEOUT =====
dnl =========================================================================

CPPFLAGS_OLD="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $BMI_CPPFLAGS"
AC_MSG_CHECKING([If BMI has support for BMI_ZOID_POST_TIMEOUT])
AC_TRY_COMPILE(
           [#include <bmi.h>],
           [BMI_set_info(0,BMI_ZOID_POST_TIMEOUT,0);],
           [
              AC_DEFINE([HAVE_BMI_ZOID_TIMEOUT],[1],[If BMI has BMI_ZOID_POST_TIMEOUT])
              AC_MSG_RESULT([yes])
           ],
           [ AC_MSG_RESULT([no])]
          )
CPPLAGS="$CPPFLAGS_OLD"

dnl =========================================================================
dnl ========= Check for ZLib support (for hash functions) ===================
dnl =========================================================================

ZLIB_LDFLAGS=
ZLIB_CFLAGS=
ZLIB_CPPFLAGS=
ZLIB_LIBS=
PKG_CHECK_MODULES([ZLib],[zlib],[use_zlib=1],[use_zlib=0])
ZLIB_LDFLAGS="$ZLib_LDFLAGS"
ZLIB_CFLAGS="$ZLib_CFLAGS"
ZLIB_CPPFLAGS="$ZLib_CPPFLAGS"
ZLIB_LIBS="$ZLib_LIBS"
AC_SUBST([ZLIB_LDFLAGS])
AC_SUBST([ZLIB_CFLAGS])
AC_SUBST([ZLIB_CPPFLAGS])
AC_SUBST([ZLIB_LIBS])
AM_CONDITIONAL([HAVE_ZLIB],[test "x${use_zlib}" == "x1"])
if test "x${use_zlib}" == "x1" ; then
      AC_DEFINE([HAVE_ZLIB], [1], [If zlib is available])
      CPPFLAGS="$CPPFLAGS $ZLib_CPPFLAGS"
      LDFLAGS="$LDFLAGS $ZLib_LDFLAGS"
      CFLAGS="$CFLAGS $ZLib_CFLAGS"
      LIBS="$LIBS $ZLib_LIBS"
      IOFSL_CLIENT_CFLAGS_PKG="${IOFSL_CLIENT_CFLAGS_PKG} ${ZLib_CFLAGS} ${ZLib_CPPFLAGS}"
      IOFSL_CLIENT_LIBS_PKG="${IOFSL_CLIENT_LIBS_PKG} ${ZLib_LDFLAGS} ${ZLib_LIBS}"
fi

dnl =========================================================================
dnl ====== BZip2 Support ====================================================
dnl =========================================================================

AC_ARG_WITH(bzip2,[  --with-bzip2=<location of bzip2 installation>
                      ],BZIP2_DIR="$withval",BZIP2_DIR="")
if test -n "$BZIP2_DIR" ;
then
   CPPFLAGS="$CPPFLAGS -I${BZIP2_DIR}/include"
   LDFLAGS="$LDFLAGS -L${BZIP2_DIR}/lib"
   IOFSL_CLIENT_CFLAGS_PKG="${IOFSL_CLIENT_CFLAGS_PKG} -I${BZIP2_DIR}/include"
   IOFSL_CLIENT_LIBS_PKG="${IOFSL_CLIENT_LIBS_PKG} -L${BZIP2_DIR}/lib -lbz2"
fi

use_bzlib=1

AC_CHECK_HEADERS([bzlib.h],[],
                 [use_bzlib=])

if test -n $use_bzlib; then
AC_SEARCH_LIBS([BZ2_bzCompress],[bz2],[],
               [use_bzlib=])
fi

AM_CONDITIONAL([HAVE_BZLIB],[test "x${use_bzlib}" == "x1"])

AC_MSG_CHECKING([if BZip2 support can be enabled])
if test "x${use_bzlib}" == "x1" ; then
      AC_DEFINE([HAVE_BZLIB], [1], [If bzlib is available])
      AC_MSG_RESULT([yes])
else
      AC_MSG_RESULT([no])
fi

dnl =========================================================================
dnl ========= Check for OpenSSL support (for hash functions) ================
dnl =========================================================================

OPENSSL_LDFLAGS=
OPENSSL_CFLAGS=
OPENSSL_CPPFLAGS=
OPENSSL_LIBS=
PKG_CHECK_MODULES([OpenSSL],[openssl],[use_openssl=1],[use_openssl=0])
OPENSSL_LDFLAGS="$OpenSSL_LDFLAGS"
OPENSSL_CFLAGS="$OpenSSL_CFLAGS"
OPENSSL_CPPFLAGS="$OpenSSL_CPPFLAGS"
OPENSSL_LIBS="$OpenSSL_LIBS"
AC_SUBST([OPENSSL_LDFLAGS])
AC_SUBST([OPENSSL_CFLAGS])
AC_SUBST([OPENSSL_CPPFLAGS])
AC_SUBST([OPENSSL_LIBS])
AM_CONDITIONAL([HAVE_OPENSSL],[test "x${use_openssl}" == "x1"])
if test "x${use_openssl}" == "x1" ; then
      AC_DEFINE([HAVE_OPENSSL], [1], [If OpenSSL is available])
      CPPFLAGS="$CPPFLAGS $OpenSSL_CPPFLAGS"
      LDFLAGS="$LDFLAGS $OpenSSL_LDFLAGS"
      CFLAGS="$CFLAGS $OpenSSL_CFLAGS"
      LIBS="$LIBS $OpenSSL_LIBS"
      IOFSL_CLIENT_CFLAGS_PKG="${IOFSL_CLIENT_CFLAGS_PKG} ${OPENSSL_CFLAGS}"
      IOFSL_CLIENT_LIBS_PKG="${IOFSL_CLIENT_LIBS_PKG} ${OPENSSL_LDFLAGS} ${OPENSSL_LIBS}"
fi

dnl =========================================================================
dnl =========================================================================
dnl =========================================================================

#
# export the client lib and cflags for pkg-config
#
AC_SUBST([IOFSL_CLIENT_CFLAGS_PKG])
AC_SUBST([IOFSL_CLIENT_LIBS_PKG])

AC_CONFIG_FILES([
                 Makefile
                 iofsl.pc
                 iofslrouter.pc
                 ])

AC_OUTPUT

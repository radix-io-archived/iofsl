dnl 
dnl Allow the user to specify   --without-pvfs2 / --with-pvfs2=dir
dnl Defines HAVE_PVFS2 if PVFS2 is available
dnl   sets PVFS2_OK to 1 if available
dnl   PVFS2_LDFLAGS PVFS2_CPPFLAGS PVFS2_CFLAG
dnl
AC_DEFUN([AX_LIB_PVFS2],
[
AC_ARG_WITH([pvfs2],
  [AC_HELP_STRING([--with-pvfs2@<:@=ARG@:>@],
         [Include PVFS2 backend support @<:@default=auto@:>@])],
  [use_pvfs2=${withval}],
  [use_pvfs2=auto])
PVFS2_LDFLAGS=""
PVFS2_CFLAGS=""
PVFS2_CPPFLAGS=""
PVFS2_OK=""
if test x${use_pvfs2} != xno
then
   found="yes"
   pvfs2_config=""
   if test "x${use_pvfs2}" != xauto -a x${use_pvfs2} != xyes
   then
      pvfs2_config="${use_pvfs2}/bin/pvfs2-config"
   else
      AC_PATH_PROG([pvfs2_config],[pvfs2-config])
   fi
      
   if test -z "${pvfs2_config}" || test ! -x ${pvfs2_config} 
   then
     dnl unless autodetection is on, fail if we cannot find pvfs2-config
     if test "x${use_pvfs2}" != "xauto" && "x${found}" == "xno"
     then
        AC_MSG_ERROR([Could not find pvfs2-config: use --with-pvfs2=pvfs2dir])
     fi
   else
     PVFS2_LDFLAGS="`${pvfs2_config} --static-libs`"
     PVFS2_LIBS="`${pvfs2_config} --libs`"
     PVFS2_CFLAGS="`${pvfs2_config} --cflags`"

     OLDCFLAGS="$CFLAGS"
     OLDCPPFLAGS="$CPPFLAGS"
     OLDLDFLAGS="$LDFLAGS"
     OLDLIBS="$LIBS"
     CFLAGS="$CPPFLAGS $PVFS2_CFLAGS"
     LDFLAGS="$LDFLAGS $PVFS2_LDFLAGS"
     LIBS="$LIBS $PVFS2_LIBSS"
     AC_CHECK_LIB([pvfs2], [PVFS_sys_readdir],[pvfs2_ok=1],[pvfs2_ok=0])
     if test "x${pvfs2_ok}" != "x1" 
     then
        if test "x${use_pvfs2}" != "xauto"
        then
           AC_MSG_ERROR([PVFS2 link test failed...])
        fi
        PVFS2_CPPFLAGS=""
        PVFS2_LDFLAGS=""
        PVFS2_LIBS=""
        PVFS2_CFLAGS=""
     else
        AC_DEFINE([HAVE_PVFS2],[1],[If PVFS2 support should be enabled])
        PVFS2_OK=1
     fi
     CPPFLAGS="$OLDCPPFLAGS"
     LDFLAGS="$OLDLDFLAGS"
     LIBS="$OLDLIBS"
     CFLAGS="$OLDCFLAGS"
   fi
fi
AC_SUBST([PVFS2_LDFLAGS])
AC_SUBST([PVFS2_LIBS])
AC_SUBST([PVFS2_CPPFLAGS])
AC_SUBST([PVFS2_CFLAGS])
])

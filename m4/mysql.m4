dnl
dnl Allows user to specify where to find mysql
dnl   sets MYSQL_CFLAGS, MYSQL_CPPFLAGS, MYSQL_LDFLAGS,
dnl   MYSQL_VERSION and MYSQL_LDFLAGS_R
dnl
dnl defines HAVE_MYSQL if found
dnl AX_LIB_MYSQL([action-if-found],[action-if-not-found])
dnl
AC_DEFUN([AX_LIB_MYSQL],
[
AC_ARG_WITH([mysql],
  AC_HELP_STRING([--with-mysql@<:@=ARG@:>@],
      [use MySQL client library @<:@default=yes@:>@, optionally specify path to mysql_config]
  ),
  [
  if test "$withval" = "no"
  then
      need_mysql="no"
  elif test "$withval" = "yes"
  then
      need_mysql="yes"
  else
      need_mysql="yes"
      MYSQL_CONFIG="$withval"
  fi
  ], [need_mysql="yes"]
)

MYSQL_CFLAGS=""
MYSQL_CPPFLAGS=""
MYSQL_LDFLAGS=""
MYSQL_LDFLAGS_R=""
MYSQL_VERSION=""

if test "$need_mysql" = "yes"
then
  if test -z "$MYSQL_CONFIG" 
  then
      AC_PATH_PROG([MYSQL_CONFIG], [mysql_config], [missing])
  fi
  AC_MSG_CHECKING([for MySQL])
  if test "$MYSQL_CONFIG" != "missing"
  then
      MYSQL_CFLAGS="`$MYSQL_CONFIG --include`"
      MYSQL_CFLAGS="`$MYSQL_CONFIG --cflags`"
      MYSQL_LDFLAGS="`$MYSQL_CONFIG --libs`"
      MYSQL_LDFLAGS_R="`$MYSQL_CONFIG --libs_r`"
      MYSQL_VERSION="`$MYSQL_CONFIG --version`"

      AC_DEFINE([HAVE_MYSQL], [1], [if MySQL is available])
      AC_MSG_RESULT([yes])
      $1
  else
      AC_MSG_RESULT([no])
      $2
  fi

AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_CPPFLAGS)
AC_SUBST(MYSQL_LDFLAGS)
AC_SUBST(MYSQL_LDFLAGS_R)
AC_SUBST(MYSQL_VERSION)

fi
])


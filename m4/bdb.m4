dnl
dnl Set BDB_LDFLAGS, BDB_CFLAGS, BDB_LIBS
dnl BDB_OK=1 if DB was found
dnl
AC_DEFUN([AX_LIB_DB],
[
    dbpath=ifelse([$1], ,,$1)

    BDB_LDFLAGS=
    BDB_CFLAGS=
    BDB_LIBS=
    BDB_OK=
    dnl 
    dnl if the db is specified, try to link with -ldb
    dnl otherwise try -ldb4, then -ldb3, then -ldb
    dnl $lib set to notfound on link failure
    dnl    
    AC_MSG_CHECKING([for db library])

    if test "x$dbpath" != "x" ; then
	oldcflags=$CFLAGS
        oldldflags=$LDFLAGS
        oldlibs=$LIBS
	for dbheader in db4 db3 notfound; do
		AC_COMPILE_IFELSE(
			[#include "$dbpath/include/$dbheader/db.h"],
			[BDB_CFLAGS="-I$dbpath/include/$dbheader/"
			 break])
	done

	if test "x$dbheader" = "xnotfound"; then
		AC_COMPILE_IFELSE(
			[#include "$dbpath/include/db.h"],
			[BDB_CFLAGS="-I$dbpath/include/"],
			[AC_MSG_ERROR(
				Invalid libdb path specified. No db.h found.)])
	fi

        BDB_LDFLAGS="-L${dbpath}/lib"
	LDFLAGS="$BDB_LDFLAGS ${LDFLAGS}"

	LIBS="${oldlibs} -ldb -lpthread"
	BDB_LIBS="-ldb"
	CFLAGS="$BDB_CFLAGS $oldcflags"
	AC_TRY_LINK(
		[#include <db.h>],
		[DB *dbp; db_create(&dbp, NULL, 0);],
		lib=db)
	CFLAGS=$oldcflags
        LDFLAGS=$oldldflags
        LIBS=$oldlibs
    else
        oldlibs=$LIBS
        for lib in db4  db3  db  notfound; do
           LIBS="${oldlibs} -l$lib -lpthread"
           BDB_LIBS="-l$lib"
           AC_TRY_LINK(
                  [#include <db.h>],
                  [DB *dbp; db_create(&dbp, NULL, 0);],
                  [break])
        done
        LIBS=$oldlibs
    fi

    if test "x$lib" = "xnotfound" ; then
           AC_MSG_RESULT(could not find DB libraries)
           BDB_OK=""
    else
           BDB_OK=1
           AC_MSG_RESULT($lib)
    fi
    AC_SUBST(BDB_CFLAGS)	
    AC_SUBST(BDB_LIBS)
    AC_SUBST(BDB_LDFLAGS)
    AC_SUBST(BDB_OK)
])

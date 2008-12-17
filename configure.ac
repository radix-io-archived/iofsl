#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([src/iofwd/iofwd.c])
AC_CONFIG_HEADER([iofwd_config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.
AC_CHECK_LIB([bmi], [BMI_initialize])
AC_CHECK_LIB([pthread], [pthread_exit])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT32_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

# find out some basic paths

AC_CONFIG_FILES([
                 Makefile
                 sandbox/module.mk
                 src/backend/module.mk
                 src/frontend/module.mk
                 src/iofwd/module.mk
                 ])

AC_OUTPUT

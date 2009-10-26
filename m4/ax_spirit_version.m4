#
# autoconf macro to determine which spirit headers need to be used.
#
# This macro calls:
#   
#   AC_SUBST([BOOST_SPIRIT_OLD])
# 
# if the classic spirit headers have been moved into the include/classic
# subdirectory.
#
AC_DEFUN([AX_BOOST_SPIRIT_OLD],
[
       AC_CHECK_HEADER([boost/spirit/include/classic_core.hpp],
         [], [AC_DEFINE([BOOST_SPIRIT_OLD],[],
          [Define to include the pre 1.36 spirit headers])])
])


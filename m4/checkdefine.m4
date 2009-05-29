AC_DEFUN([AC_CHECK_HEADER_DEFINE],[
AS_VAR_PUSHDEF([ac_var],[ac_cv_header_define_$2])dnl
AC_CACHE_CHECK([if $2 is defined in $1], ac_var,
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include $1]],[[
if ($2)
{
}
]])],[AS_VAR_SET(ac_var, yes)],[AS_VAR_SET(ac_var, no)]))
AS_IF([test AS_VAR_GET(ac_var) != "no"], [$3], [$4])dnl
AS_VAR_POPDEF([ac_var])dnl
])



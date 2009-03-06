AC_DEFUN([AX_MAKE_VAR],
[
MY_MAKE_VARS="${MY_MAKE_VARS} $1"
])

AC_DEFUN([AX_OUTPUT_MAKE_VARS],
[
MAKE_VARS=""
for var in ${MY_MAKE_VARS}
do
eval MAKE_VARS=\"\${MAKE_VARS}${var}=\${$var}\"
MAKE_VARS="${MAKE_VARS}\\n"
done
MAKE_VARS="$(echo -e ${MAKE_VARS})"
AC_SUBST([MAKE_VARS])
])

#!/bin/sh

if test -z "${IOFWD_SRCDIR}" ; then
   echo "Need IOFWD_SRCDIR !"
   exit 1
fi

SCRIPTDIR="${IOFWD_SRCDIR}/scripts"
CONFIG="${SCRIPTDIR}/scripts/configoptions.$(hostname)"
LOGDIR="${IOFWD_SRCDIR}/buildlogs"
CURDIR="$PWD"
REV="$1"
LOGFILE="${LOGDIR}/runtest-rev.log"
REVLOG="${LOGDIR}/${REV}.log"

if test -z "${REV}" ; then
   echo "No revision specified!"
fi

if test -r "${CONFIG}" ; then
   source "${CONFIG}" || exit 1
fi

cd "${IOFWD_SRCDIR}" || exit 1
FULLLOGMSG="$(git log --format=oneline ${REV}^..${REV})"
test "$?" == "0" || exit 1

LOGMSG="$(echo "${FULLLOGMSG}" | cut -c 42-)"
REV="$(echo "${FULLLOGMSG}" | cut -c 1-40)" 

TEMPDIR="$(mktemp -d --tmpdir build-${REV}.XXX)"

git archive "${REV}" | tar -x -C "$TEMPDIR" || exit 1

mkdir -p "${LOGDIR}"

cd "${TEMPDIR}"
echo "Building ${REV} (${LOGMSG})"
${SCRIPTDIR}/runtest.sh 2>&1 | tee "${REVLOG}"
ret=$?
STATUS="FAIL"
if test "${ret}" == "0" ; then
   STATUS="OK  "
fi

echo "${REV} ${STATUS} $(date) ${LOGMSG}" >> "${LOGFILE}"

exit "${ret}"

#!/bin/sh

# test revisions before pushing

if test -z "${IOFWD_SRCDIR}" ; then
   echo "Need IOFWD_SRCDIR!"
   exit 1
fi

${IOFWD_SRCDIR}/scripts/runtest-range.sh "origin/master.."


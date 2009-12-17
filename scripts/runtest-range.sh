#!/bin/sh

if test -z "${IOFWD_SRCDIR}" ; then
   echo "Need IOFWD_SRCDIR !"
   exit 1
fi

SCRIPTDIR="${IOFWD_SRCDIR}/scripts"

git log --format=oneline $1 | tac | cut -d' ' -f1 \
        | while read rev
do
   ${SCRIPTDIR}/runtest-rev.sh "${rev}"
   if test "$?" != "0" ; then
      ERRORLOG="${IOFWD_SRCDIR}/buildlogs/${rev}.log"
      echo 
      echo '************************************************'
      echo '********** runtest-rev FAILED ******************'
      echo '************************************************'
      echo 
      echo "Failed revision was ${rev}"
      echo 
      echo "Errorlog: ${ERRORLOG}"
      tail ${ERRORLOG}
      exit 1
   fi
done

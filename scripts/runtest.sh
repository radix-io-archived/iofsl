#!/bin/sh

#
# Prepares, builds and tests 
#

SRCDIR=$PWD
if test -z "${IOFWD_SRCDIR}" ; then
   echo "Need IOFWD_SRCDIR ! (to find configoptions)"
   exit 1
fi


SCRIPTDIR="${IOFWD_SRCDIR}/scripts"
CONFIGFILE="${SCRIPTDIR}/configoptions.$(hostname)"
if test -r "${CONFIGFILE}" ; then
   source  "${CONFIGFILE}"
fi

cd ${SRCDIR}
./prepare || exit 1

${SRCDIR}/configure ${DISTCHECK_CONFIGURE_FLAGS} || exit 2
make distcheck || exit 3


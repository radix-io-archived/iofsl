#!/bin/sh

#
# Prepares, builds and tests 
#

if test -z "${IOFWD_SRCDIR}" ; then
   echo "Need IOFWD_SRCDIR !"
   exit 1
fi


SCRIPTDIR="${IOFWD_SRCDIR}/scripts"
CONFIGFILE="${SCRIPTDIR}/configoptions.$(hostname)"
if test -r "${CONFIGFILE}" ; then
   source  "${CONFIGFILE}"
fi

OLDDIR=$(pwd)
./prepare || exit 1
cd ${OLDDIR}

${IOFWD_SRCDIR}/configure ${DISTCHECK_CONFIGURE_FLAGS} || exit 2
make distcheck || exit 3


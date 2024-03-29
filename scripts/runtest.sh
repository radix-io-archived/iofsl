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
if test -r "${CONFIGFILE}" ; echo "config file is $CONFIGFILE" ; then
   source  "${CONFIGFILE}" ; echo "configure options = $CONFIGURE_OPTIONS" 
else echo "could not use config file"
fi

OLDDIR="${PWD}"
cd ${SRCDIR}
./prepare || exit 1

${SRCDIR}/configure ${DISTCHECK_CONFIGURE_FLAGS} || exit 2
make distcheck || exit 3
cd "${OLDDIR}"



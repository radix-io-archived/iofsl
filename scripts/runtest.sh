#!/bin/sh

#
# Prepares, builds and tests 
#
IOFWD_SRCDIR=/homes/rjdamore/iofsl
SRCDIR=$PWD
if test -z "${IOFWD_SRCDIR}" ; then
   echo "Need IOFWD_SRCDIR ! (to find configoptions)"
   exit 1
fi


SCRIPTDIR="${IOFWD_SRCDIR}/scripts"
CONFIGFILE="${SCRIPTDIR}/configoptions.$(hostname)"
if test -r "${CONFIGFILE}" ; echo "config file is $CONFIGFILE.$(hostname)" ; then
   source  "${CONFIGFILE}" ; echo "configure options = $CONFIGURE_OPTIONS" 
else echo "could not use config file"
fi

cd ${SRCDIR}
./prepare || exit 1


${SRCDIR}/configure ${CONFIGURE_OPTIONS} || exit 2
make distcheck  || exit 3


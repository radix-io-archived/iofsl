#!/bin/sh

#
# Configures source tree in IOFWD_SRCDIR; Will configure in current
# directory.
#

if test -z "${IOFWD_SRCDIR}" ; then
   echo "Need IOFWD_SRCDIR !"
   exit 1
fi

CONFIG="${IOFWD_SRCDIR}/scripts/configoptions.$(hostname)"

if test -r "${CONFIG}" ; then
   source "${CONFIG}"
fi

${IOFWD_SRCDIR}/configure ${CONFIGURE_OPTIONS} \
       CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS" $*


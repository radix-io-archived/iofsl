#!/bin/bash
# This script can be used to trigger the nightly build scripts locally. No
# guarantees!

if test -z "$1" -o -z "$2" ; then
   echo "Need two arguments: path to source tree and path to build dir"
   exit 1
fi

export SRCDIR="$1"
export WORKSPACE="$2"

echo "IOFSL source in $SRCDIR"
echo "Building     in $WORKSPACE"

exec "${SRCDIR}/autobuild/build.sh"


#!/bin/sh

if test -z "${IOFWD_SRCDIR}" ; then
   DIR=$PWD
else
   DIR=${IOFWD_SRCDIR}
fi

cd $DIR
find . -iname \*.cpp -o -iname \*.hh | xargs grep "$*"

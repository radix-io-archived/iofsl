#!/bin/sh

SOURCE_DIRS=src 
export CHECK_SCRIPTS=$IOFWD_SRCDIR/test/check-scripts

find ${SOURCE_DIRS} \( -iname \*.cpp -o -iname \*.c \) -print0 \
   | xargs -0 -L1 ${CHECK_SCRIPTS}/runcppcheck.sh


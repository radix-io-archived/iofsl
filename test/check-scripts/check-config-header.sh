#!/bin/sh
FILE=$1

grep '#include.*' $1 | head -1 | sed -e 's/\/\/.*//' | grep 'iofsl-global.h' > \
   /dev/null
OK=$?

if test $OK != 0; then
   if test -z $FIXME; then
     echo "File $FILE did not include iofsl-global.hh as first header!"
     echo "Rerun with FIXME exported to automatically correct..."
     exit 1
   else
     echo "Fixing file $FILE... Adding needed #include"
     awk 'BEGIN { print "#include \"iofwd-global.h\"" } { print; }' \
        $FILE > $FILE.tmp
     mv $FILE.tmp $FILE
   fi
fi

exit 0

#!/bin/bash

# backup the file
cp $1 $1.old

# replace single spaces and tabs, write new file to .new
sed 's/[ \t]*$//' $1 > $1.new

# if the sed command failed, revert to old file
if [ $? != 0 ] ; then
    cp $1.old $1
# if sed command passed, update existing file to modified file
else
    cp $1.new $1
fi

# cleanup the tmp files
rm $1.old $1.new

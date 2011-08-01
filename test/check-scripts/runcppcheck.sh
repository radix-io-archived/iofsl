#!/bin/sh
echo $*
exec ${CHECK_SCRIPTS}/check-config-header.sh $*


#!/bin/sh

#
# Generate new daily build if there was a change to the git tree
#
#

# location of the checkout out git tree
GITDIR=/tmp/iofsl-daily

# URL to clone
GITURL=git://git.mcs.anl.gov/iofsl

# branch to watch
BRANCH=origin/master

changed=0

if ! test -d "${GITDIR}" ; then
   git clone ${GITURL} ${GITDIR} || exit 1
   changed=1
else
   cd ${GITDIR}
   git remote update || exit 1
   count=$(git rev-list HEAD..${BRANCH} --count)
   if test 0 -lt "${count}" ; then
      changed=1
   fi
fi

if test "$changed" != "1"; then
   exit 0
fi

echo "Need to build daily build"

cd ${GITDIR}
git pull --rebase




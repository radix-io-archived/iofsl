#!/bin/bash

# Helper functions for autobuild
#
#   Assumes AUTOBUILDDIR is set and valid
#

function do_error ()
{
   echo "Error: $1" >2
   exit 1
}

test -n "${AUTOBUILDDIR}" || do_error "Need AUTOBUILDDIR"


# Global Settings

# Fetch files described in $1 into dir $2, checking cache in $3
function fetch_deps () {
   FETCH="$1"
   OUTPUTDIR="$2"
   CACHEDIR="$3"
   cat "${FETCH}" | grep -v '^#' | while read checksum output url
   do
      if test -z "${url}"; then
         continue
      fi

      echo -n "${output}: "
      CACHED="${CACHEDIR}/${output}"
      OUTPUTNAME="${OUTPUTDIR}/${output}"

      # Check if file is readable, owned by us and regular
      if test -r "${CACHED}" -a -f "${CACHED}" -a -O "${CACHED}"; then
         cachesum=$(md5sum "${CACHED}" | cut -f1 -d' ')
         if test "A${checksum}" = "A${cachesum}" ; then
            echo -n "(found in cache) "
         else
            echo -n "(cached but invalid. Removing) "
            rm "${CACHED}" || exit 2
         fi
      fi

      if ! test -r "${CACHED}"; then
         echo -n "(fetching) "
         # file doesn't exist; fetch it
         rm -f "${CACHED}"
         wget -q -O "${CACHED}" "${url}" || exit 1
      fi


      # copy into dest
      cp -f "${CACHED}" "${OUTPUTNAME}" || exit 1

      # verify md5sum
      cachesum=$(md5sum "${OUTPUTNAME}" | cut -f1 -d' ')
      if test "A${checksum}" != "A${cachesum}" ; then
         echo "Incorrect checksum (expecting ${checksum}, got ${cachesum}"
         echo "Aborting!"
         exit 2
      fi
      echo "OK (${checksum})"
   done
}

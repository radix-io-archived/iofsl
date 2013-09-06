#!/bin/bash

if test -z "${WORKSPACE}" -o -z "${BUILD_ID}" ; then
   echo "This script can only be ran under jenkins"
   exit 1
fi

# Dump some information
ls ${WORKSPACE}

echo "======================================================================="
echo "= Build ${BUILD_NUMBER} ${BUILD_ID}"
echo "=    on ${NODE_NAME} [${NODE_LABELS}]"
echo "======================================================================="


BUILD="${WORKSPACE}/build"
mkdir -p "${BUILD}"

"${WORKSPACE}/autobuild/build-local.sh" "${WORKSPACE}" "${BUILD}"



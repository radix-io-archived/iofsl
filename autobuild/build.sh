#!/bin/bash

# Build script
ls ${WORKSPACE}

AUTOBUILDDIR=${WORKSPACE}/autobuild

# Dump some information
echo "======================================================================="
echo "= Build ${BUILD_NUMBER} ${BUILD_ID}"
echo "=    on ${NODE_NAME} [${NODE_LABELS}]"
echo "======================================================================="

echo
echo "======================================================================="
echo "==== STEP 1: Fetch build dependencies ================================="
echo "======================================================================="




# force failure for now
exit 1

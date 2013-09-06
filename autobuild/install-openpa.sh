#!/bin/bash

echo "Building OpenPA"

DEPBUILDDIR="${1}/openpa"
INPUT="$2"
PREFIX="$3"

echo $*

mkdir -p "${DEPBUILDDIR}"
cd "${DEPBUILDDIR}"
tar --strip-components 1 -C "${DEPBUILDDIR}" -xzf "${INPUT}" || exit 1

cd "${DEPBUILDDIR}"
./configure --prefix="${PREFIX}" && make && make install


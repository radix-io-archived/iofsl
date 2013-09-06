#!/bin/bash


echo "Building BOOST"

DEPBUILDDIR="${1}/boost"
INPUT="$2"
PREFIX="$3"

LIBS="atomic,chrono,date_time,exception,filesystem,iostreams,program_options,random,regex,system,test,thread,timer,wave"

mkdir -p "${DEPBUILDDIR}"

cd "${DEPBUILDDIR}"

tar --strip-components 1 -C "${DEPBUILDDIR}" -xf "${INPUT}"

cd "${DEPBUILDDIR}"
./bootstrap.sh --prefix="$PREFIX" --with-libraries="${LIBS}"

./b2 install

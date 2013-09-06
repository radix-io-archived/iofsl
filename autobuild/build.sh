#!/bin/bash

# Build script
ls ${WORKSPACE}

AUTOBUILDDIR=${WORKSPACE}/autobuild

# ===============  Validate environment ===================

if test -z "${SRCDIR}"; then
   echo "This script requires SRCDIR to be defined"
   echo "(and pointing to the IOFSL source tree)"
   exit 1
fi

# Make sure srcdir is absolute
tmp=$(echo -n ${SRCDIR} | cut -c1-2)
if test "A$tmp" != "A/" ; then
   # make srcdir absolute
   SRCDIR=$(cd "${SRCDIR}" && pwd)
fi

if ! test -r "${SRCDIR}/iofsl.pc.in" ; then
   echo "Doesn't look like ${SRCDIR} contains an IOFSL source tree"
   exit 1
fi


if test -z "${WORKSPACE}" -a -d "${WORKSPACE}"; then
   echo "This scripts requires WORKSPACE to be defined (and exist)"
   exit 1
fi

# ====================================================================
# ========== Settings ================================================
# ====================================================================

# where we'll fetch dependencies and other files needed for the build
FETCHDIR="${WORKSPACE}/fetch-files"

# Where we'll cache build info (fetched files, ccache, etc.)
CACHEDIR="/tmp/iofsl-cache"

# Dir containing build scripts
AUTOBUILDDIR=${SRCDIR}/autobuild

# ====================================================================
# ========= Dump some settings =======================================
# ====================================================================

echo "===================================================================="
echo "===== INFO ========================================================="
echo "===================================================================="

echo "IOFSL Source tree : ${SRCDIR}"
echo "Workspace         : ${WORKSPACE}"
echo "Cachedir          : ${CACHEDIR}"

# ====================================================================
# Load rest of build function
# ====================================================================

if ! test -r "${AUTOBUILDDIR}/functions.sh" ; then
   echo "Missing ${AUTOBUILDDIR}/functions.sh!"
   exit 1
fi

source "${AUTOBUILDDIR}/functions.sh" || exit 2


# ===================================================================
# ==== Internal variables | DO NOT MODIFY ===========================
# ===================================================================

# list of files we will fetch
FETCH="${AUTOBUILDDIR}/files.fetch"

# Where we'll install dependencies
DEPINSTALL="${WORKSPACE}/install"

# Where we'll build IOFSL
BUILDDIR="${WORKSPACE}/build"


# ===================================================================
# ==== Validate internal environment ================================
# ===================================================================

if  ! test -d "${CACHEDIR}" -o ! -O "${CACHEDIR}" -o ! -G "${CACHEDIR}" ; then
   echo "Invalid cachedir ${CACHEDIR}; Doesn't exist or not owned by build"
fi


echo
echo "======================================================================="
echo "==== STEP 1: Fetch build dependencies ================================="
echo "======================================================================="

mkdir -p "${FETCHDIR}"
mkdir -p "${CACHEDIR}"

fetch_deps "${FETCH}" "${FETCHDIR}" "${CACHEDIR}"  || exit 1

echo "======================================================================="
echo "==== STEP 2: Build & install dependencies ============================="
echo "======================================================================="

DEPBUILDDIR="${WORKSPACE}/depbuild"
mkdir -p "${DEPBUILDDIR}"
mkdir -p "${DEPINSTALL}"



echo -n " * Building and installing boost..."
"${AUTOBUILDDIR}/install-boost.sh" \
   "${DEPBUILDDIR}" \
   "${FETCHDIR}/boost.tar.bz2" \
   "${DEPINSTALL}" \
   > "${DEPBUILDDIR}/boost-log" \
    || exit 2
echo DONE

echo -n " * Building and installing openpa..."
"${AUTOBUILDDIR}/install-openpa.sh" \
   "${DEPBUILDDIR}" \
   "${FETCHDIR}/openpa.tar.gz" \
   "${DEPINSTALL}" \
   > "${DEPBUILDDIR}/openpa-log" \
   || exit 2
echo DONE

echo -n " * Building and installing BMI..."
"${AUTOBUILDDIR}/install-bmi.sh" \
   "${DEPBUILDDIR}" \
   "${FETCHDIR}/bmi.tar.gz" \
   "${DEPINSTALL}" \
   > "${DEPBUILDDIR}/bmi-log" \
   || exit 2
echo DONE

echo "======================================================================="
echo "===== STEP 3: Configure IOFSL ========================================="
echo "======================================================================="

BUILDDIR="${WORKSPACE}/iofslbuild"
INSTALLDIR="${WORKSPACE}/iofslinstall"

mkdir -p "${BUILDDIR}"

echo "* Generating buildsystem"
cd "${SRCDIR}"
./prepare

echo "* Configuring IOFSL"
CONFIGOPTS="--prefix=${INSTALLDIR} --with-bmi=${DEPINSTALL}"
cd "${BUILDDIR}"
"${SRCDIR}/configure" ${CONFIGOPTS} || exit 3

echo "======================================================================="
echo "==== STEP 4: Building IOFSL ==========================================="
echo "======================================================================="
make -j2 || exit 4

echo "======================================================================="
echo "==== STEP 5: Install IOFSL ============================================"
echo "======================================================================="
make install || exit 5

echo "======================================================================="
echo "===== STEP 6: Distcheck ==============================================="
echo "======================================================================="
make distcheck DISTCHECK_CONFIGURE_FLAGS="${CONFIGOPTS}" || exit 6


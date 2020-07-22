#!/bin/sh

set -euo pipefail

# set help message
usage="usage: [NUM_PROCS=<num-procs>] create_external_tarball.sh <branch> [<tmp-dir>]"

# show help message if no arguments are given or too many
if [ "${#}" == 0 ]
then
    echo "${usage}"
    exit 0
elif [ "${#}" -gt 2 ]
then
    echo "Too many arguments!"
    echo "${usage}"
    exit 1
fi

# set working directory
workdir="$(pwd)"
testlog="${workdir}/test_$(date '+%F_%H-%M').log"

# assign command line parameters and define default values
branch=${1}
tmpdir=${2:-${workdir}/tmp}

# determine number of processes to use
procs=$(nproc)
if ! [ -z ${NUM_PROCS+x} ]
then
    procs=${NUM_PROCS}
fi

# verify that the temporary folder does not exist
if [ -d ${tmpdir} ]
then
    echo "FAILURE: Temporary folder '${tmpdir}' already exists!!!"
    exit 1
fi

# directories
repodir="${tmpdir}/gpispace/${branch}"
builddir="${repodir}/build"
installdir="${builddir}/install"
testdir="${builddir}/test-dir"

# package source code & cleanup temporary files on exit
success=false
function package_and_cleanup()
{
    if ${success}
    then
        tar --exclude ${builddir} -czf ${workdir}/gpispace-${branch}.tar.gz -C ${tmpdir} .
        rm ${testlog}
        echo "SUCCESS"
    else
        echo "ABORTED"
    fi

    killall -u $(id -un) -qs SIGKILL gpi-space gspc-rifd agent drts-kernel gspc-logging-demultiplexer.exe || true
    rm -rf "${tmpdir}"
}

trap package_and_cleanup EXIT

# create temporary directory
mkdir -p ${repodir}
cd ${repodir}

# clone git repo
git clone \
    --recurse-submodules \
    -j8 \
    --branch ${branch} \
    git@gitlab.hpc.devnet.itwm.fhg.de:top/gpispace.git \
    ${repodir}

# patch sources for external tarball
cmake \
    -D BRANCH=${branch} \
    -P .internal/external.cmake
git apply external.patch

# cleanup repo
git rm -r \
    .ci \
    .docker \
    .gitlab-ci.yml \
    .internal \
    .mailmap \
    CHANGELOG.todo \
    doc/intern \
    playground \
    tools
git submodule foreach --quiet 'rm $toplevel/$path/.git'
rm -rf \
    .git \
    .gitmodules \
    external.patch

# build & test
mkdir -p ${testdir}
cd ${builddir}

# configure
cmake -DCMAKE_BUILD_TYPE:STRING=Release \
    -DCMAKE_INSTALL_PREFIX:PATH="${installdir}" \
    -DSHARED_DIRECTORY_FOR_TESTS:PATH="${testdir}" \
    -DTESTING_RIF_STRATEGY:STRING="ssh" \
    -DFHG_ASSERT_MODE:BOOL=1 \
    ..

# build
cmake --build . -- -j ${procs}

# install
cmake --install .

# test
function run_tests()
{
    hostname > "${GSPC_NODEFILE_FOR_TESTS}"
    ctest \
        --output-on-failure \
        --timeout 180 \
        --schedule-random \
        -LE "performance_test" \
        -j ${procs} \
        -VV \
        --output-log ${testlog}
}
GSPC_NODEFILE_FOR_TESTS="${builddir}/nodefile" run_tests

success=true

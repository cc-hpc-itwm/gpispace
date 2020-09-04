#! /bin/bash

set -euo pipefail

# set help message
usage="usage: [NUM_PROCS=<num-procs>] create_external_tarball.sh <branch> [<tmp-dir>] [<output-file>]"

# show help message if no arguments are given or too many
if [ "${#}" == 0 ]
then
    echo "${usage}"
    exit 0
elif [ "${#}" -gt 3 ]
then
    echo "Too many arguments!" >&2
    echo "${usage}" >&2
    exit 1
fi

# set working directory
workdir="$(readlink -f "$(pwd)")"
testlog="${workdir}/test_$(date '+%F_%H-%M').log"

# assign command line parameters and define default values
branch="${1}"
tmpdir="$(readlink -f "${2:-${workdir}/tmp}")"
output_file="${3:-${workdir}/gpispace-${branch}.tar.gz}"

# determine number of processes to use
procs="${NUM_PROCS:-$(nproc)}"

# verify that the temporary folder does not exist
if [ -d "${tmpdir}" ]
then
    echo "FAILURE: Temporary folder '${tmpdir}' already exists!!!" >&2
    exit 1
fi

if test -e "${workdir}/gpispace-${branch}.tar.gz"
then
  echo "FAILURE: output file ('${output_file}') already exists." >&2
  exit 2
fi

# directories
repodir="${tmpdir}/gpispace/${branch}"
builddir="${tmpdir}/build"
installdir="${builddir}/install"
testdir="${builddir}/test-dir"

# package source code & cleanup temporary files on exit
success=false
function package_and_cleanup()
{
    if "${success}"
    then
        tar -czf "${output_file}" -C "${tmpdir}" gpispace
        rm "${testlog}"
        echo "SUCCESS"
    else
        echo "ABORTED"
    fi

    killall -u "$(id -un)" -qs SIGKILL gpi-space gspc-rifd agent drts-kernel gspc-logging-demultiplexer.exe || true
    rm -rf "${tmpdir}"
}

trap package_and_cleanup EXIT

# create temporary directory
mkdir -p "${repodir}"
cd "${repodir}"

# clone git repo
git clone \
    --recurse-submodules \
    -j8 \
    --branch "${branch}" \
    git@gitlab.hpc.devnet.itwm.fhg.de:top/gpispace.git \
    "${repodir}"

# patch sources for external tarball
cmake \
    -D BRANCH="${branch}" \
    -P .internal/external.cmake
git apply external.patch

# patch doc/readme/ to be assembled in-tarball
cp "${repodir}/.internal/external/doc/readme/CMakeLists.txt" \
   "${repodir}/doc/readme/CMakeLists.txt"
"${repodir}/doc/readme/configure.sh" \
  "${repodir}/doc/readme/parts" \
  "${repodir}/doc/readme/parts/readme-full" \
  "${repodir}/doc/readme/getting_started.md"
"${repodir}/doc/readme/configure.sh" \
  "${repodir}/doc/readme/parts" \
  "${repodir}/doc/readme/parts/driver-full" \
  "${repodir}/doc/readme/driver.cpp"
"${repodir}/doc/readme/configure.sh" \
  "${repodir}/doc/readme/parts" \
  "${repodir}/doc/readme/parts/script-full" \
  "${repodir}/doc/readme/build.sh"
"${repodir}/doc/readme/configure.sh" \
  "${repodir}/doc/readme/parts" \
  "${repodir}/doc/readme/parts/pnet-full" \
  "${repodir}/doc/readme/compute_and_aggregate.xpnet"
rm -rf "${repodir}/doc/readme/parts"

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
    share/xsd/Makefile \
    share/xsd/XSDtoRNG.xsl \
    share/xsd/pnet.rnc.fix.patch \
    src/sdpa/test/sdpa/Scheduler.performance.*.DAT \
    tools
git submodule foreach 'git rm -r .ci .gitlab-ci.yml || true'
git submodule foreach --quiet 'rm "${toplevel}/${path}/.git"'
rm -rf \
    .git \
    .gitmodules \
    external.patch

# soak up input before writing to allow in-place awk
function sponge()
{
  local output="${1}"
  perl -e "@lines = <>; open OUT, '>${output}' or die; print OUT @lines or die"
}

function add_license_header_comment()
{
  local style="${1}"
  local pattern="${2}"

  while read -r file
  do
    awk \
      -v style="${style}" \
      -v year="$(date +%Y)" \
      'FNR == 1 {
         firstline = $0
         getline

         emptyfmt = style == "c" ? "//" \
                  : style == "bash" ? "#" \
                  : style == "man" ? ".\\\"" \
                  : style == "xml" ? "<!--                                                                       -->" \
                  : "<unknown style>"
         fmt = style == "xml" ? "<!-- %-69s -->" \
             : emptyfmt " %s"
         surround_with_whitespace = style != "man"

         if (firstline ~ /^(#!|<\?xml)/) {
           print firstline
           if (surround_with_whitespace) print ""
         }

         printf (fmt "\n", "This file is part of GPI-Space.")
         printf (fmt "\n", "Copyright (C) " year " Fraunhofer ITWM")
         printf (emptyfmt "\n")
         printf (fmt "\n", "This program is free software: you can redistribute it and/or modify")
         printf (fmt "\n", "it under the terms of the GNU General Public License as published by")
         printf (fmt "\n", "the Free Software Foundation, either version 3 of the License, or")
         printf (fmt "\n", "(at your option) any later version.")
         printf (emptyfmt "\n")
         printf (fmt "\n", "This program is distributed in the hope that it will be useful,")
         printf (fmt "\n", "but WITHOUT ANY WARRANTY; without even the implied warranty of")
         printf (fmt "\n", "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the")
         printf (fmt "\n", "GNU General Public License for more details.")
         printf (emptyfmt "\n")
         printf (fmt "\n", "You should have received a copy of the GNU General Public License")
         printf (fmt "\n", "along with this program. If not, see <https://www.gnu.org/licenses/>.")

         if (firstline ~ /^(#!|<\?xml)/) {
           if (surround_with_whitespace && $0 !~ /^$/) print ""
         } else {
           if (surround_with_whitespace && firstline !~ /^$/) print ""
           print firstline
         }
       }
       FNR != 1 {
         print
       }' "${file}" | sponge "${file}"
  done < <(find ./* \
                -regextype posix-extended \
                -regex "${pattern}" \
                -and -not -path './external/beegfs-client-devel*' \
                -and -not -path './external/rapidxml*' \
                -and -not -path './src/xml/tests/xpnets*.xpnet' \
                -type f \
          )
}

add_license_header_comment 'c' '.*\.(cpp(.in|)|hpp(.in|)|ipp|h)'
add_license_header_comment 'bash' '.*(\.(cmake|rnc|sh)|CMakeLists.txt|Makefile)'
add_license_header_comment 'xml' '.*\.(xpnet(.in|)|xml|xsd)'
add_license_header_comment 'man' 'share/man/.*\.[0-9].*'

# build & test
mkdir -p "${testdir}"
cd "${builddir}"

# configure
cmake -DCMAKE_BUILD_TYPE:STRING=Release \
    -DCMAKE_INSTALL_PREFIX:PATH="${installdir}" \
    -DSHARED_DIRECTORY_FOR_TESTS:PATH="${testdir}" \
    -DTESTING_RIF_STRATEGY:STRING="ssh" \
    -DFHG_ASSERT_MODE:BOOL=1 \
    "${repodir}"

# build
cmake --build . -- -j "${procs}"

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
        -j "${procs}" \
        -VV \
        --output-log "${testlog}"
}
GSPC_NODEFILE_FOR_TESTS="${builddir}/nodefile" run_tests

success=true

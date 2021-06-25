#!/bin/bash

# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

set -euo pipefail
scriptfile="$(readlink -f ${BASH_SOURCE})"; scriptdir="${scriptfile%/*}"
cd "${scriptdir}"

if test ${#} -lt 1
then
  echo >&2 "usage: ${0} GPISPACE_INSTALL_DIR [APP_INSTALL_DIR [LOG_PORT [CXX \
[RIF_STRATEGY [extra_runtime_system_arguments]...]]]]"
  exit 1
fi

GPISPACE_INSTALL_DIR="${1}"

if test -z "${2:-}"
then
  APP_INSTALL_DIR="$(mktemp -d)"
  trap 'rm -rv "${APP_INSTALL_DIR}"' EXIT
else
  APP_INSTALL_DIR="${2}"
fi

if test ! -d "${APP_INSTALL_DIR}"
then
  echo >&2 "${0}: Error: APP_INSTALL_DIR '${APP_INSTALL_DIR}' does not exist."
  exit 1
fi

for dir in bin lib src compute_and_aggregate.pnet nodefile
do
  if test -e "${APP_INSTALL_DIR}/${dir}"
  then
    echo >&2 "${0}: Error: APP_INSTALL_DIR '${APP_INSTALL_DIR}' must not contain any of {bin,lib,src,compute_and_aggregate.pnet,nodefile}."
    exit 1
  fi
done

LOG_PORT="${3:-}"
CXX="${4:-c++}"
RIF_STRATEGY="${5:-ssh}"
shift 5 || true

mkdir -p "${APP_INSTALL_DIR}/"{bin,lib,src}

"${GPISPACE_INSTALL_DIR}/bin/pnetc"                               \
  --input "compute_and_aggregate.xpnet"                           \
  --output "${APP_INSTALL_DIR}/compute_and_aggregate.pnet"

"${GPISPACE_INSTALL_DIR}/bin/pnetc"                               \
  --input="compute_and_aggregate.xpnet"                           \
  --output="/dev/null"                                            \
  --gen-cxxflags=-O3                                              \
  --path-to-cpp="${APP_INSTALL_DIR}/src"

make install                                                      \
  -C "${APP_INSTALL_DIR}/src"                                     \
  LIB_DESTDIR="${APP_INSTALL_DIR}/lib"

"${CXX}"                                                          \
  -Wall -Wextra -Werror                                           \
  -std=c++11                                                      \
  -DAPP_INSTALL_DIR="\"${APP_INSTALL_DIR}\""                      \
  -DGPISPACE_INSTALL_DIR="\"${GPISPACE_INSTALL_DIR}\""            \
  driver.cpp                                                      \
  -o "${APP_INSTALL_DIR}/bin/compute_and_aggregate"               \
                                                                  \
  -isystem "${GPISPACE_INSTALL_DIR}/include"                      \
  -L "${GPISPACE_INSTALL_DIR}/lib/"                               \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/lib/"                       \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/libexec/bundle/lib/"        \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/libexec/iml/"               \
  -lgspc                                                          \
                                                                  \
  -isystem "${GPISPACE_INSTALL_DIR}/external/boost/include"       \
  -L "${GPISPACE_INSTALL_DIR}/external/boost/lib/"                \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/external/boost/lib/"        \
  -Wl,--exclude-libs,libboost_program_options.a                   \
  -lboost_program_options                                         \
  -lboost_filesystem                                              \
  -lboost_system

if test -n "${LOG_PORT}"
then
"${GPISPACE_INSTALL_DIR}/bin/gspc-monitor" --port "${LOG_PORT}" &
fi

hostname > "${APP_INSTALL_DIR}/nodefile"
# note: the location doesn't matter for the execution
# note: this script puts the nodefile into the ${APP_INSTALL_DIR} as
# this directory is known to be writeable
# note: to test in a cluster allocation, for `--nodefile` below, use
# Slurm: "$(generate_pbs_nodefile)"
# PBS/Torque: "${PBS_NODEFILE}"

"${APP_INSTALL_DIR}/bin/compute_and_aggregate"                    \
  --rif-strategy "${RIF_STRATEGY:-ssh}"                           \
  --nodefile "${APP_INSTALL_DIR}/nodefile"                        \
  ${LOG_PORT:+--log-host ${HOSTNAME} --log-port ${LOG_PORT}}      \
  --N 20                                                          \
  --workers-per-node 4                                            \
  "${@}"

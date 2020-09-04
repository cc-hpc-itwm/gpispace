#!/bin/bash

# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
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

GPISPACE_INSTALL_DIR="${1}"
APP_INSTALL_DIR="${2:-$(mktemp -d)}"
LOG_PORT="${3:-}"
CXX="${4:-c++}"

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
  -isystem "${GPISPACE_INSTALL_DIR}/include"                      \
  -isystem "${GPISPACE_INSTALL_DIR}/external/boost/include"       \
  -Wl,--exclude-libs,libboost_program_options.a                   \
  -L "${GPISPACE_INSTALL_DIR}/lib/"                               \
  -L "${GPISPACE_INSTALL_DIR}/external/boost/lib/"                \
  -Wl,-rpath-link,"${GPISPACE_INSTALL_DIR}/external/boost/lib/"   \
  -Wl,-rpath,"${GPISPACE_INSTALL_DIR}/lib:${GPISPACE_INSTALL_DIR}/external/boost/lib/" \
  driver.cpp                                                      \
  -lgspc                                                          \
  -lboost_program_options                                         \
  -lboost_system                                                  \
  -o "${APP_INSTALL_DIR}/bin/compute_and_aggregate"

if test -n "${LOG_PORT}"
then
"${GPISPACE_INSTALL_DIR}/bin/gspc-monitor" --port "${LOG_PORT}" &
fi

hostname > nodefile
# or to test in a cluster allocation, for `--nodefile` below, use
# Slurm: "$(generate_pbs_nodefile)"
# PBS/Torque: "${PBS_NODEFILE}"

"${APP_INSTALL_DIR}/bin/compute_and_aggregate"                    \
  --rif-strategy ssh                                              \
  --nodefile "${PWD}/nodefile"                                    \
  ${LOG_PORT:+--log-host ${HOSTNAME} --log-port ${LOG_PORT}}      \
  --N 20                                                          \
  --workers-per-node 4

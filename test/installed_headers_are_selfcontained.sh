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

install_prefix=${1:?missing argument 1: install_prefix}
cxx_compiler=${2:?missing argument 2: cxx_compiler}
num_parallel_procs=${3:-10}

cxx_args="-x c++ -o /dev/null -c --std=c++14"
cxx_args="${cxx_args} -I ${install_prefix}/include"
cxx_args="${cxx_args} -I ${install_prefix}/external/boost/include"

width=$(cd "${install_prefix}/include"; find * -type f | sed -e 's,.,X,g' | sort | tail -n 1 | wc -c)

process_pool_locks=()
outputs=()
pids=()
trap 'pkill -P $$ || true; rm -f ${outputs[@]}; rm -f ${process_pool_locks[@]}' EXIT

for ((i = 0; i < ${num_parallel_procs}; ++i))
do
  process_pool_locks+=("$(mktemp)")
done

lock_id=0
while read path
do
  output=$(mktemp)
  outputs+=("${output}")

  echo "${path}" > "${output}"

  flock --exclusive "${process_pool_locks[${lock_id}]}" \
        "${cxx_compiler}" ${cxx_args} - \
        <<<"#include <${path}>" \
        >>"${output}" 2>&1 \
        &
  pids+=(${!})

  lock_id=$(((lock_id + 1) % ${#process_pool_locks[@]}))
done < <(cd "${install_prefix}/include"; find * -type f | grep -v 'ipp$')

ec=0
for ((i = 0; i < ${#pids[@]}; ++i))
do
  printf "%-${width}s " "$(head -n 1 ${outputs[i]}) "
  if ! wait ${pids[i]}
  then
    echo FAIL
    tail -n +2 ${outputs[i]} \
      | (grep -v 'In file included from <stdin>:1:0:' || true) \
      | (grep -v 'compilation terminated.' || true)
    ec=1
  else
    echo OK
  fi
done
exit ${ec}

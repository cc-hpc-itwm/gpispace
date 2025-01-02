#!/bin/bash

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

install_prefix=${1:?missing argument 1: install_prefix}
cxx_compiler=${2:?missing argument 2: cxx_compiler}
cxx_args=("${3}")
num_parallel_procs=${4:-10}

cxx_args+=("-x" "c++")
cxx_args+=("-o" "/dev/null")
cxx_args+=("-c")
cxx_args+=("-E")
cxx_args+=("--std=c++17")
cxx_args+=("-I" "${install_prefix}/include")
cxx_args+=("-I" "${install_prefix}/external/boost/include")

width=$(cd "${install_prefix}/include"; find -name "*" -type f -not -path "*/util-qt/*" | sed -e 's,.,X,g' | sort | tail -n 1 | wc -c)

process_pool_locks=()
outputs=()
pids=()
trap 'pkill -P $$ || true; rm -f ${outputs[@]}; rm -f ${process_pool_locks[@]}' EXIT

for ((i = 0; i < "${num_parallel_procs}"; ++i))
do
  process_pool_locks+=("$(mktemp)")
done

lock_id=0
while read -r path
do
  output=$(mktemp)
  outputs+=("${output}")

  echo "${path}" > "${output}"

  flock --exclusive "${process_pool_locks[${lock_id}]}" \
        "${cxx_compiler}" "${cxx_args[@]}" - \
        <<<"#include <${path}>" \
        >>"${output}" 2>&1 \
        &
  pids+=(${!})

  lock_id=$(((lock_id + 1) % ${#process_pool_locks[@]}))
done < <(cd "${install_prefix}/include"; find -name "*" -type f -not -path "*/util-qt/*" | grep -v 'ipp$')

ec=0
for ((i = 0; i < ${#pids[@]}; ++i))
do
  printf "%-${width}s " "$(head -n 1 "${outputs[i]}") "
  if ! wait "${pids[i]}"
  then
    echo FAIL
    tail -n +2 "${outputs[i]}" \
      | (grep -v 'In file included from <stdin>:1:0:' || true) \
      | (grep -v 'compilation terminated.' || true)
    ec=1
  else
    echo OK
  fi
done
exit "${ec}"

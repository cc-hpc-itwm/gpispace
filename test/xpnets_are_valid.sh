#!/bin/bash

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

repo_dir=${1:?missing argument 1: repo directory}
xmllint=${2:?missing argument 2: xmllint}
num_parallel_procs=${3:-10}

function find_xpnets()
{
  # intended to fail parsing
  # - src/xml/tests/xpnets/diagnostics/error_duplicate_template.xpnet

  # not actually an xpnet
  # - install/share/GPISpace/xml/xsd/schemas.xml (ci installs into source dir)
  # - share/xml/xsd/schemas.xml

  # in CI the build is in the PROJECT_DIRECTORY and it contains
  # 3rdparty non-xpnet xml files

  (cd "${repo_dir}" && find * -name '*.xpnet' -or -name '*.xml' -type f) \
    | grep -v '3rdparty' \
    | grep -v 'install/share/GPISpace/xml/xsd/schemas.xml$' \
    | grep -v 'share/xml/xsd/schemas.xml$' \
    | grep -v 'src/xml/tests/xpnets/diagnostics/error_duplicate_template.xpnet$'
}

width=$(find_xpnets | sed -e 's,.,X,g' | sort | tail -n 1 | wc -c)

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
        "${xmllint}" --noout --schema "${repo_dir}/share/xml/xsd/pnet.xsd" \
        "${repo_dir}/${path}" \
        >>"${output}" 2>&1 \
        &
  pids+=(${!})

  lock_id=$(((lock_id + 1) % ${#process_pool_locks[@]}))
done < <(find_xpnets)

ec=0
for ((i = 0; i < ${#pids[@]}; ++i))
do
  printf "%-${width}s " "$(head -n 1 ${outputs[i]}) "
  if ! wait ${pids[i]}
  then
    echo FAIL
    tail -n +2 ${outputs[i]}
    ec=1
  else
    echo OK
  fi
done
exit ${ec}

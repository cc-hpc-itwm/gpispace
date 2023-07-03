#!/bin/bash

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

parts_dir="${1:?missing argument 1: parts_dir}"; shift
input="${1:?missing argument 2: input}"; shift
output="${1:?missing argument 3: output}"; shift
dependencies=("${@:-}")

state="$(mktemp)"
trap 'rm "${state}"' EXIT

function contains () {
  local e match="${1}"
  shift
  for e; do [[ "${e}" == "${match}" ]] && return 0; done
  return 1
}

cat "${input}" > "${state}"
while grep -q "@@.*@@" "${state}"
do
  for pattern in $(grep -o "@@.*@@" "${state}" | sort -u)
  do
    part_file=${parts_dir}/${pattern//@/}
    if ! test -f "${part_file}"
    then
        echo >&2 "$0: Error: Missing file '${part_file}'"
        exit 1
    fi
    if ! contains "${pattern//@/}" "${dependencies[@]}"
    then
        echo >&2 "$0: Error: Missing dependency '${part_file}'"
        exit 1
    fi
    sed -i'' \
        -e "/${pattern}/r ${part_file}" \
        -e "/${pattern}/d" \
        "${state}"
  done
done

cat "${state}" > "${output}"

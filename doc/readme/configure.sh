#!/bin/bash
set -euo pipefail

parts_dir="${1}"
input="${2}"
output="${3}"

state="$(mktemp)"
trap 'rm "${state}"' EXIT

cat "${input}" > "${state}"
while grep -q "@@.*@@" "${state}"
do
  for pattern in $(grep -o "@@.*@@" "${state}" | sort -u)
  do
    test -f "${parts_dir}/${pattern//@/}"
    sed -i'' \
        -e "/${pattern}/r ${parts_dir}/${pattern//@/}" \
        -e "/${pattern}/d" \
        "${state}"
  done
done

cat "${state}" > "${output}"

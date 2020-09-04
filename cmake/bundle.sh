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

destination="${1}"
chrpath_binary="${2}"
input="${3}"

function join { local IFS="$1"; shift; echo "${*}"; }
exclusion=$(join '|' \
  .*libibverbs\.so.* \
  .*libxcb\.so.* \
  .*libSM\.so.* \
  .*libc\.so.* \
  .*libz\.so.* \
  .*libm\.so.* \
  .*librt\.so.* \
  .*libfont\.so.* \
  .*libfreetype\.so.* \
  .*libaudio\.so.* \
  .*libICE\.so.* \
  .*libglapi\.so.* \
  .*libglib\.so.* \
  .*libgobject\.so.* \
  .*libdl\.so.* \
  .*libX.*\.so.* \
  .*libGL\.so.* \
  .*libpthread\.so.* \
  .*libgthread\.so.* \
  .*libreadline\.so.* \
)

rm -rf "${destination}"
mkdir -p "${destination}"

did_not_fail=false
function remove_output_on_error
{
  if ! ${did_not_fail}
  then
    rm -rf "${destination}"
  fi
}
trap remove_output_on_error EXIT

for dependency_and_path in $( LD_BIND_NOW=1 ldd "${input}" \
                            | grep -vE "${exclusion}" \
                            | grep '=> \(not\|/\)' \
                            | awk '{printf("%s:%s\n", $1, $3)}' \
                            )
do
  dependency="${dependency_and_path%:*}"
  path="${dependency_and_path#*:}"

  if [ "${path}" = "not" ]
  then
    echo >&2 "cannot resolve dependency '${dependency}'"
    exit 2
  fi

  dest="$(readlink -f "${destination}/${dependency}")"
  cp "$(readlink -f "${path}")" "${dest}"
  chmod +w "${dest}"

  "${chrpath_binary}" -d "${dest}"
done

did_not_fail=true

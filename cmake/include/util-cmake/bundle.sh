#!/bin/bash

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later


#! This script is not intended to be used stand-alone but is used by

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! `add_macros.cmake` to find dynamic libraries to bundle into a

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! location and host independent bundle for deployment.

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later


set -euo pipefail

destination="${1}"
chrpath_binary="${2}"
input="${3}"

# Assumption: `$input` in the current state either has rpath or
# `$LD_LIBRARY_PATH` set up to actually find all dependencies. The
# script errors if this is not the case.

# Note: This script has no relation to the linking steps, so
# e.g. `-Lfoo -la` would like to bundle `foo/liba.so`, but if
# `bundle.sh` is called with `LD_LIBRARY_PATH=bar` and `bar/liba.so`
# exists, `bar/liba.so` will be bundled, which probably leads to
# undefined behavior. CMake will set the "build rpath" to include all
# paths for libaries linked to avoid this.

function join { local IFS="$1"; shift; echo "${*}"; }

# Don't bundle system libraries that aren't portable.
exclusion=$(join '|' \
  '.*libGL\.so.*' \
  '.*libICE\.so.*' \
  '.*libSM\.so.*' \
  '.*libX.*\.so.*' \
  '.*libaudio\.so.*' \
  '.*libfont\.so.*' \
  '.*libfreetype\.so.*' \
  '.*libglapi\.so.*' \
  '.*libglib\.so.*' \
  '.*libgobject\.so.*' \
  '.*libgthread\.so.*' \
  '.*libibverbs\.so.*' \
  '.*libreadline\.so.*' \
  '.*libxcb\.so.*' \
  '.*libz\.so.*' \
)

## glibc differs too much between distros to be freely copied, but
## does exist essentially everywhere.
exclusion=${exclusion}\|$(join '|' \
  '.*libBrokenLocale\.so.*' \
  '.*libSegFault\.so.*' \
  '.*libanl\.so.*' \
  '.*libc\.so.*' \
  '.*libcidn\.so.*' \
  '.*libcrypt\.so.*' \
  '.*libdl\.so.*' \
  '.*libm\.so.*' \
  '.*libmemusage\.so.*' \
  '.*libnsl\.so.*' \
  '.*libnss_compat\.so.*' \
  '.*libnss_db\.so.*' \
  '.*libnss_dns\.so.*' \
  '.*libnss_files\.so.*' \
  '.*libnss_hesiod\.so.*' \
  '.*libnss_nis\.so.*' \
  '.*libnss_nisplus\.so.*' \
  '.*libpcprofile\.so.*' \
  '.*libpthread\.so.*' \
  '.*libresolv\.so.*' \
  '.*librt\.so.*' \
  '.*librtkaio\.so.*' \
  '.*libthread_db\.so.*' \
  '.*libutil\.so.*' \
)

# Don't bundle products that rely on secondary files in their
# installation that don't show up in ldd.

## GPI-Space
exclusion=${exclusion}\|$(join '|' \
  'libdrts-context\.so' \
  'libgspc\.so' \
  'libwe-dev\.so'
)

## IML
exclusion=${exclusion}\|$(join '|' \
  'libIML-Client\.so' \
  'libIMLPrivate-Installation\.so' \
)

## \todo Qt? See #9 and #22.
#exclusion="${exclusion}|.*libQt.*\.so.*"

rm -rf "${destination}"
mkdir -p "${destination}"

success=false
function remove_output_on_error
{
  if ! ${success}
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

success=true

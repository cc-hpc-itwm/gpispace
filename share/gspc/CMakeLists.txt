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

set (licenses_filename "${CMAKE_CURRENT_BINARY_DIR}/licenses.txt")

file (WRITE "${licenses_filename}" "")
macro (append_license _package _license _extra _licensetextname)
  file (APPEND "${licenses_filename}"
    "--- ${_package} -----------------------------------------------------------

This software uses ${_package} which is licensed under ${_license}.
")
  if (_extra)
    file (APPEND "${licenses_filename}" "${_extra}
")
  endif()
  file (APPEND "${licenses_filename}"
"
${LICENSE_${_licensetextname}}
")
endmacro()


file (READ licenses/lgpl-3.0.txt LICENSE_LGPLv3)
foreach (name "Qt 5")
  append_license ("${name}"
    "the GNU Lesser General Public License version 3 (LGPLv3.0)"
    "Source code for the library can be obtained by contacting
gpispace-support@itwm.fraunhofer.de."
    LGPLv3
  )
endforeach()

file (READ licenses/lgpl-2.1.txt LICENSE_LGPLv2)
foreach (name "ibverbs" "libnuma")
  append_license ("${name}"
    "the GNU Lesser General Public License version 2.1 (LGPLv2.1)"
    "Source code for the library can be obtained by contacting
gpispace-support@itwm.fraunhofer.de."
    LGPLv2
  )
endforeach()

file (READ licenses/bsl_1.0.txt LICENSE_Boost)
foreach (name "Boost" "rapidxml")
  append_license ("${name}" "the Boost Software License - Version 1.0" "" Boost)
endforeach()

file (READ licenses/bsd_hwloc.txt LICENSE_BSD_hwloc)
append_license ("hwloc" "a BSD License" "" BSD_hwloc)

file (READ licenses/bsd_libssh2.txt LICENSE_BSD_libssh2)
append_license ("libssh2" "a BSD License" "" BSD_libssh2)

file (READ licenses/openssl.txt LICENSE_openssl)
append_license ("openssl" "the OpenSSL license" "" openssl)

file (READ licenses/zlib.txt LICENSE_zlib)
append_license ("zlib" "following license" "" zlib)

# beegfs: omit due to being in-house and not distributed
# gpi-2: omit due to being in-house

install (FILES "${licenses_filename}" DESTINATION share/gspc)

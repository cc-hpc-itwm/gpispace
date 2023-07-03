# This file assembles `share/GPISpace/LICENSE`.

macro (add_license _name _license)
  file (READ "${_license}" _tmp)
  string (PREPEND _tmp
"
===============================================================================

${_name} License

===============================================================================

"
)
  string (APPEND _license_file "${_tmp}")
endmacro()

add_license (GPI-Space ${PROJECT_SOURCE_DIR}/LICENSE)

add_license (Boost Boost/LICENSE)
add_license (HWLOC HWLOC/LICENSE)
add_license (LibNUMA LibNUMA/LICENSE)
add_license (Libssh2 Libssh2/LICENSE)
add_license (OpenSSL OpenSSL/LICENSE)
add_license (Qt5 Qt5/LICENSE)
add_license (RapidXML RapidXML/LICENSE)
add_license (ZLib ZLib/LICENSE)
add_license (ibverbs ibverbs/LICENSE)
# beegfs: omit due to being in-house and not distributed
# gpi-2: omit due to being in-house

set (_licenses_filename
  "${CMAKE_CURRENT_BINARY_DIR}/LICENSES"
)
file (WRITE
  "${_licenses_filename}"
  "${_license_file}"
)

install (
  FILES
    "${_licenses_filename}"
  DESTINATION
    "${GSPC_SHARE_INSTALL_DIR}"
)

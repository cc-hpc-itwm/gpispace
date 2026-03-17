install (
  FILES
    "${PROJECT_SOURCE_DIR}/LICENSE"
  DESTINATION
    "${GSPC_SHARE_INSTALL_DIR}"
)

install (
  DIRECTORY
    beegfs
    Boost
    GPI2
    HWLOC
    LibNUMA
    LibRT
    $<$<BOOL:${GSPC_WITH_EXPRESSION_SHELL}>:LibReadline>
    Libssh2
    OpenSSL
    Qt5
    RapidXML
    ZLib
    fmt
    ibverbs
  DESTINATION
    "${GSPC_SHARE_INSTALL_DIR}/licenses"
)

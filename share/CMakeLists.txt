add_subdirectory (licenses)

install (
  DIRECTORY
    doc
    man
    selftest
    xml
  DESTINATION
    ${GSPC_SHARE_INSTALL_DIR}
  USE_SOURCE_PERMISSIONS
)

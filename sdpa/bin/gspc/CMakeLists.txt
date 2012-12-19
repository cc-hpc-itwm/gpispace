configure_file(gspc-sh-version.in ${CMAKE_CURRENT_BINARY_DIR}/gspc-sh-version @ONLY)

set (prgs
  gspc
)

set (scripts
  gspc-rexec
  gspc-config
  gspc-list-commands
  gspc-check-port
  gspc-shell
  gspc-help
  gspc-template
  gspc-topology
  gspc-topology-status
  gspc-topology-list
  gspc-log
  gspc-init
  gspc-kvs
)

set (files
  gspc-sh-setup
  ${CMAKE_CURRENT_BINARY_DIR}/gspc-sh-version
  )

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT runtime)

install(PROGRAMS ${scripts}
  DESTINATION libexec/gspc/scripts
  COMPONENT runtime)

install (FILES ${files}
  DESTINATION libexec/gspc/scripts
  COMPONENT runtime)

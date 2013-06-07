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
  gspc-topology-start
  gspc-log
  gspc-init
  gspc-kvs
  gspc-version

  gspc-net

  gspc-rif
  gspc-rif-bootstrap
  gspc-rif-shutdown
  gspc-rif-raw

  gspc-ctrl
  gspc-ctrl-unload-modules
  gspc-ctrl-current-job
  gspc-ctrl-logcat
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

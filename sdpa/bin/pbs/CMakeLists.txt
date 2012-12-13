configure_file(gspc-sh-version.in ${CMAKE_CURRENT_BINARY_DIR}/gspc-sh-version @ONLY)

set (prgs
  sdpa
  gspc
)

set (scripts
  clear-logs
  start-gpi
  start-gui
  start-orch
  start-agent
  start-drts
  start-sdpa
  stop-drts
  stop-agent
  status-agent
  status-drts
  pidof

  gspc-rexec
  gspc-config
  gspc-list-commands
  gspc-check-port
  gspc-shell
  gspc-help
  gspc-template
  gspc-topology
)

set (files
  gspc-sh-setup
  ${CMAKE_CURRENT_BINARY_DIR}/gspc-sh-version
  )

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT runtime)

install(PROGRAMS ${scripts}
  DESTINATION libexec/sdpa/scripts
  COMPONENT runtime)

install (FILES ${files}
  DESTINATION libexec/sdpa/scripts
  COMPONENT runtime)

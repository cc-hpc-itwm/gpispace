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
  gspc-sh-setup
)

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT runtime)

install(PROGRAMS ${scripts}
  DESTINATION libexec/sdpa/scripts
  COMPONENT runtime)

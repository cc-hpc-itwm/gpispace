set (prgs
  sdpa
)

set (scripts
  clear-logs
  start-gpi
  start-gui
  start-orch
  start-agent
  start-drts
  stop-drts
  stop-agent
  status-agent
  status-drts
  pidof
)

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT runtime)

install(PROGRAMS ${scripts}
  DESTINATION libexec/sdpa/scripts
  COMPONENT runtime)

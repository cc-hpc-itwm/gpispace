set (prgs
  sdpa
)

set (scripts
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
  sdpa-selftest
)

install(PROGRAMS ${prgs}
  DESTINATION bin
)

install(PROGRAMS ${scripts}
  DESTINATION libexec/sdpa/scripts
)

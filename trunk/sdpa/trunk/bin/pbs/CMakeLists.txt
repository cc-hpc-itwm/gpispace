set (prgs
  clear-logs
  init-screen
  sdpa
  start-gpi
  start-gui
  start-orch
  start-agent
  start-drts
  stop-drts
  stop-agent
  send-signal
  pidof
)

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT applications)

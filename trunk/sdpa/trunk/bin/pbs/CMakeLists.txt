set (prgs
  clear-logs
  init-screen
  sdpa
  start-agg
  start-gpi
  start-gui
  start-nre
  start-orch
  start-pc
  start-drts
  start-agent
  stop-nre
  stop-pc
  stop-drts
  send-signal
  pidof
)

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT applications)

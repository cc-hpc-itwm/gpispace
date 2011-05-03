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
  stop-nre
  stop-pc
  send-signal
)

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT applications)

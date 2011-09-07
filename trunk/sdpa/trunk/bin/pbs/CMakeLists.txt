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
  stop-nre
  stop-pc
  send-signal
  pidof
)

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT applications)

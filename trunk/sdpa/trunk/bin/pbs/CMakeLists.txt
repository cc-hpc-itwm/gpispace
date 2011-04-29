set (prgs
  clear-logs
  init-screen
  sdpa
  start-agg
  start-gpi
  start-nre
  start-orch
  start-pc
  stop-nre
  stop-pc
)

install(PROGRAMS ${prgs}
  DESTINATION bin/pbs
  COMPONENT applications)

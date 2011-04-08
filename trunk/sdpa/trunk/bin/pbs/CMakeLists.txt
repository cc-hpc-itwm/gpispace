set (prgs
  sdpa
  start-agg
  start-orch
  start-nre
  start-gpi
  kill-nre
  boot-nre
  check-nodes
  status-nre
  fvm-cleanup
  init-screen
  clear-logs
  ipc-cleanup
  stop-nre
  stop-pc
  start-pc
)

install(PROGRAMS ${prgs}
  DESTINATION bin/pbs
  COMPONENT applications)

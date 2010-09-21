set (prgs
  start-nre
  start-agg
  start-orch
  kill-nre
  boot-nre
  check-nodes
  status-nre
  fvm-cleanup
  init-screen
  )

install(PROGRAMS ${prgs}
  DESTINATION bin/pbs
  COMPONENT applications)

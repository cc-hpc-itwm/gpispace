configure_file (${CMAKE_CURRENT_SOURCE_DIR}/start-nre.in ${CMAKE_CURRENT_BINARY_DIR}/start-nre @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/start-gpi.in ${CMAKE_CURRENT_BINARY_DIR}/start-gpi @ONLY)

set (prgs
  start-agg
  start-orch
  ${CMAKE_CURRENT_BINARY_DIR}/start-nre
  ${CMAKE_CURRENT_BINARY_DIR}/start-gpi
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

add_subdirectory(SMC)
add_dependencies(sdpa-daemonFSM_SMC sdpa-events)

if (USE_BOOST_SC)
  add_subdirectory(BSC)
  add_dependencies(sdpa-daemonFSM_BSC sdpa-events sdpa_daemon)
endif (USE_BOOST_SC)

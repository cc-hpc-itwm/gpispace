add_subdirectory(SMC)
add_dependencies(sdpa-daemonFSM_SMC sdpa-events)

if (WITH_BOOST_SC)
  add_subdirectory(BSC)
  add_dependencies(sdpa-daemonFSM_BSC sdpa-events)
endif (WITH_BOOST_SC)

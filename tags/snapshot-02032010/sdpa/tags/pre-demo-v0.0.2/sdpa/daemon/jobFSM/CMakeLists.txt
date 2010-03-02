add_subdirectory(SMC)
add_dependencies(sdpa-jobFSM_SMC sdpa-events)

if (USE_BOOST_SC)
  add_subdirectory(BSC)
  add_dependencies(sdpa-jobFSM_BSC sdpa-events)
endif (USE_BOOST_SC)

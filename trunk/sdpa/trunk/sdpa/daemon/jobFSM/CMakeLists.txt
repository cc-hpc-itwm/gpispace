if (USE_SMC_SC)
	add_subdirectory(SMC)
	add_dependencies(sdpa-jobFSM_SMC sdpa-events)
endif (USE_SMC_SC)

if (USE_BOOST_SC)
  	add_subdirectory(BSC)
  	add_dependencies(sdpa-jobFSM_BSC sdpa-events)
endif (USE_BOOST_SC)

if (USE_BOOST_MSM)
 add_subdirectory(BMSM)
endif (USE_BOOST_MSM)

set (hdrs JobFSM.hpp)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/jobFSM
  COMPONENT headers)

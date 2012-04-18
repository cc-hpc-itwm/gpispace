if (USE_BOOST_MSM)
 add_subdirectory(BMSM)
endif (USE_BOOST_MSM)

set (hdrs JobFSM.hpp)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/jobFSM
  COMPONENT headers)

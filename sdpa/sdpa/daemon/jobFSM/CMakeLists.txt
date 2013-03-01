add_subdirectory(BMSM)

set (hdrs JobFSM.hpp)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/jobFSM
  COMPONENT headers)

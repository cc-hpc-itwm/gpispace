set (hdrs
  DaemonFSM.hpp
  )

set (srcs
  DaemonFSM.cpp
  )

ADD_LIBRARY(sdpa-daemonFSM_BMSM ${srcs})
target_link_libraries(sdpa-daemonFSM_BMSM sdpa-events sdpa-daemon)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemonFSM/BMSM
  COMPONENT headers)
install(TARGETS sdpa-daemonFSM_BMSM
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)

set (hdrs
  DaemonFSM.hpp
  )

set (srcs
  Reactions.cpp
  )

ADD_LIBRARY(sdpa-daemonFSM_BSC ${srcs})
#target_link_libraries(sdpa-daemonFSM_BSC sdpa-events sdpa_daemon)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemonFSM/BSC
  COMPONENT headers)
install(TARGETS sdpa-daemonFSM_BSC
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)

set (hdrs
  JobFSM.hpp
  )

set (srcs
  Reactions.cpp
  )

ADD_LIBRARY(sdpa-jobFSM_BSC ${srcs})

install(FILES ${hdrs}
  DESTINATION include/sdpa/jobFSM/BSC
  COMPONENT headers)
install(TARGETS sdpa-jobFSM_BSC
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)

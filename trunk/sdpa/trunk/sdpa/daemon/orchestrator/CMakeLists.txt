# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
  Orchestrator.hpp
  SchedulerOrch.hpp
  )

set (srcs
  Orchestrator.cpp
)

ADD_LIBRARY(sdpa-orch ${hdrs} ${srcs})
target_link_libraries(sdpa-orch ${FhgLog_LIBRARY})

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/orchestrator
  COMPONENT headers)
  
install(TARGETS sdpa-orch
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
  
install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/orchestrator
  COMPONENT headers)


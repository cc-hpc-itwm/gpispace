# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
	AgentFactory.hpp
  	Agent.hpp
  	AgentScheduler.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/agent
  COMPONENT headers
  )


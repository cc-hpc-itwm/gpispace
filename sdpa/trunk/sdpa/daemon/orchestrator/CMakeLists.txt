# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
	OrchestratorFactory.hpp
  	Orchestrator.hpp
  	SchedulerOrch.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/orchestrator
  COMPONENT headers)


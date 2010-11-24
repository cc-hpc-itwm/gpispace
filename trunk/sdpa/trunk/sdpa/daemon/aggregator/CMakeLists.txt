# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
	AggregatorFactory
  	Aggregator.hpp
  	SchedulerAgg.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/aggregator
  COMPONENT headers
  )


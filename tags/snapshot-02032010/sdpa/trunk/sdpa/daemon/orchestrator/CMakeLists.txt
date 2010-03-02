# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

ADD_LIBRARY(sdpa-orchestrator ${hdrs} ${srcs})

if (USE_SMC_SC)
	add_dependencies(sdpa-orchestrator sdpa-daemon sdpa-daemonFSM_SMC)
	target_link_libraries(sdpa-orchestrator sdpa-daemon sdpa-daemonFSM_SMC) 
endif (USE_SMC_SC)

if (USE_BOOST_SC)
	add_dependencies(sdpa-orchestrator sdpa-daemon sdpa-daemonFSM_BSC)
	target_link_libraries(sdpa-orchestrator sdpa-daemon sdpa-daemonFSM_BSC) 
endif (USE_BOOST_SC)

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/orchestrator)
install(TARGETS sdpa-orchestrator ARCHIVE DESTINATION lib)

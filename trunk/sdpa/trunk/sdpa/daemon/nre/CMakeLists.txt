# -*- mode: cmake; -*-

file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

add_subdirectory(nre-worker)
#ADD_LIBRARY(sdpa-nre ${srcs})

#if (USE_SMC_SC)
	#add_dependencies(sdpa-nre sdpa-daemon sdpa-daemonFSM_SMC)
	#target_link_libraries(sdpa-nre sdpa-daemon sdpa-daemonFSM_SMC) 
#endif (USE_SMC_SC)

#if (USE_BOOST_SC)
#	target_link_libraries(sdpa-nre sdpa-daemon sdpa-daemonFSM_BSC) 
#endif (USE_BOOST_SC)
#target_link_libraries(sdpa-nre dl)

if (USE_BOOST_SC)
	target_link_libraries(sdpa-nre sdpa-daemon sdpa-daemonFSM_BSC) 
endif (USE_BOOST_SC)
target_link_libraries(sdpa-nre dl)

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/nre)
#install(TARGETS sdpa-nre ARCHIVE DESTINATION lib)

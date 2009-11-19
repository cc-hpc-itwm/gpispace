
file(GLOB daemonFSM-src *.cpp *.hpp)

ADD_LIBRARY(sdpa-daemonFSM_BSC ${daemonFSM-src})
target_link_libraries(sdpa-daemonFSM_BSC sdpa-events sdpa_daemon)

install(FILES ${hdrs} DESTINATION include/sdpa/daemonFSM/BSC)
install(TARGETS sdpa-daemonFSM_BSC ARCHIVE DESTINATION lib)

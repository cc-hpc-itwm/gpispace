
file(GLOB daemonFSM-src *.cpp *.hpp)

ADD_LIBRARY(sdpa-daemonFSM_BSC ${daemonFSM-src})

install(FILES ${hdrs} DESTINATION include/sdpa/daemonFSM/BSC)
install(TARGETS sdpa-daemonFSM_BSC ARCHIVE DESTINATION lib)

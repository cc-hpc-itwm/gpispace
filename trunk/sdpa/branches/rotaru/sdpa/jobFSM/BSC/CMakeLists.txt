file(GLOB jobFSM-src *.cpp *.hpp ../JobFSMInterfaceImpl.cpp)

ADD_LIBRARY(sdpa-jobFSM_BSC ${jobFSM-src})

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM/BSC)
install(TARGETS sdpa-jobFSM_BSC ARCHIVE DESTINATION lib)

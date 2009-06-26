file(GLOB jobFSM-src *.cpp *.hpp )

ADD_LIBRARY(sdpa-jobFSM_BSC ${jobFSM-src} ../JobFSMInterfaceTestImpl.cpp)

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM/BSC)
install(TARGETS sdpa-jobFSM_BSC ARCHIVE DESTINATION lib)

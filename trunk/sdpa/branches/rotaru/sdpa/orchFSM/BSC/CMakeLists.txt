
file(GLOB orchFSM-src *.cpp *.hpp ../OrchFSMInterfaceTestImpl.cpp)

ADD_LIBRARY(sdpa-orchFSM_BSC ${orchFSM-src})

install(FILES ${hdrs} DESTINATION include/sdpa/orchFSM/BSC)
install(TARGETS sdpa-orchFSM_BSC ARCHIVE DESTINATION lib)

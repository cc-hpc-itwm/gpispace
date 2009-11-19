# -*- mode: cmake; -*-

file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

ADD_LIBRARY(sdpa-nre ${hdrs} ${srcs})
target_link_libraries(sdpa-nre sdpa-daemon sdpa-daemonFSM_SMC) 

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/nre)
install(TARGETS sdpa-nre ARCHIVE DESTINATION lib)

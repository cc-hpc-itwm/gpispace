# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

ADD_LIBRARY(sdpa-aggregator ${hdrs} ${srcs})
add_dependencies(sdpa-aggregator sdpa-daemonFSM_SMC)
target_link_libraries(sdpa-aggregator sdpa-daemonFSM_SMC) 

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/aggregator)
install(TARGETS sdpa-aggregator ARCHIVE DESTINATION lib)

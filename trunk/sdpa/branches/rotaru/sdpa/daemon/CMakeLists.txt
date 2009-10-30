file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_subdirectory(orchestrator)
add_subdirectory(aggregator)
add_subdirectory(nre)

add_subdirectory(jobFSM)
add_subdirectory(daemonFSM)

ADD_LIBRARY(sdpa-daemon ${hdrs} ${srcs})
target_link_libraries(sdpa-daemon sdpa sdpa-events sdpa-jobFSM_BSC)

install(FILES ${hdrs} DESTINATION include/sdpa/daemon)
install(TARGETS sdpa-daemon ARCHIVE DESTINATION lib)

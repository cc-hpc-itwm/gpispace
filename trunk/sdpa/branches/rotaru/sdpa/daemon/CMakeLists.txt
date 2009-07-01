file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_subdirectory(orchestrator)
add_subdirectory(aggregator)
add_subdirectory(nre)


ADD_LIBRARY(sdpa-daemon ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/daemon)
install(TARGETS sdpa-daemon ARCHIVE DESTINATION lib)

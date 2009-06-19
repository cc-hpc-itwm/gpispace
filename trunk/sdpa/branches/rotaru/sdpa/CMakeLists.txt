file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_subdirectory(events)
add_subdirectory(modules)
add_subdirectory(jobFSM)
add_subdirectory(orchFSM)
add_subdirectory(orchestrator)	
add_subdirectory(aggregator)
add_subdirectory(nre)

ADD_LIBRARY(sdpa ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

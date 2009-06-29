file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_subdirectory(events)
add_subdirectory(wf)
add_subdirectory(modules)
add_subdirectory(daemon)
add_subdirectory(client)
add_subdirectory(jobFSM)
add_subdirectory(daemonFSM)

ADD_LIBRARY(sdpa ${hdrs} ${srcs})
add_dependencies(sdpa-daemon sdpa-events)
add_dependencies(sdpa sdpa-events sdpa-modules sdpa-daemon sdpa-wf)
add_dependencies(sdpa sdpa-jobFSM_BSC sdpa-daemonFSM_BSC sdpa-jobFSM_SMC sdpa-daemonFSM_SMC)

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

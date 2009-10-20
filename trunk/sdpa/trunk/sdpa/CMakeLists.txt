add_subdirectory(util)
add_subdirectory(events)
add_subdirectory(wf)
add_subdirectory(modules)
add_subdirectory(daemon)
add_subdirectory(client)

file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(sdpa ${hdrs} ${srcs})
add_dependencies(sdpa-daemon sdpa-events)
add_dependencies(sdpa sdpa-events sdpa-modules sdpa-daemon)

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

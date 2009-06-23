add_subdirectory(events)
add_subdirectory(modules)
add_subdirectory(daemon)

file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(sdpa ${hdrs} ${srcs})
add_dependencies(sdpa sdpa-events sdpa-modules sdpa-daemon)

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_subdirectory(modules)
add_subdirectory(events)
add_subdirectory(daemon)

ADD_LIBRARY(sdpa ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

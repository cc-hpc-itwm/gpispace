file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(sdpa-util ${srcs})
target_link_libraries(sdpa-util ${UUID_LIBRARY})

install(FILES ${hdrs} DESTINATION include/sdpa/util)
install(TARGETS sdpa-util ARCHIVE DESTINATION lib)

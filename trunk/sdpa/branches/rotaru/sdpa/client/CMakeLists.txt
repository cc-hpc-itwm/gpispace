file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/client)
install(TARGETS sdpa-client ARCHIVE DESTINATION lib)

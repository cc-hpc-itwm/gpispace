file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(sdpa-util ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/util)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

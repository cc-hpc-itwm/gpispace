file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_subdirectory(messages)

ADD_LIBRARY(sdpa ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

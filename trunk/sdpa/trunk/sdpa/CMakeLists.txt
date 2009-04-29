file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

ADD_LIBRARY(sdpa STATIC ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)

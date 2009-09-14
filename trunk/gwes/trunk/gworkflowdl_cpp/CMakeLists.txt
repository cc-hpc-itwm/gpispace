
file(GLOB gwdl-hdrs "include/gwdl/*.h")
file(GLOB gwdl-srcs "src/*.cpp")

# library gworkflowdl_cpp
ADD_LIBRARY(gworkflowdl_cpp ${gwdl-srcs})

install(FILES ${gwdl-hdrs} DESTINATION include/gwdl)
install(TARGETS gworkflowdl_cpp ARCHIVE DESTINATION lib)

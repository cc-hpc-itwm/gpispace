
file(GLOB hdrs "include/gwdl/*.h")
file(GLOB srcs "src/*.cpp")
#ADD_LIBRARY(gworkflowdl_cpp ${hdrs} ${srcs})
ADD_LIBRARY(gworkflowdl_cpp ${srcs})

install(FILES ${hdrs} DESTINATION include/gwdl)
install(TARGETS gworkflowdl_cpp ARCHIVE DESTINATION lib)

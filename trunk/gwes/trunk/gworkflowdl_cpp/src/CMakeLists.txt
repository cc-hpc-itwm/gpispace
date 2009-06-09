
file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
ADD_LIBRARY(gworkflowdl_cpp ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/gwdl)
install(TARGETS gworkflowdl_cpp ARCHIVE DESTINATION lib)

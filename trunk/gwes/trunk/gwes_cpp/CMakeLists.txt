
file(GLOB hdrs "include/gwdl/*.h")
file(GLOB srcs "src/*.cpp")

# library gwes_cpp
#ADD_LIBRARY(gwes_cpp ${hdrs} ${srcs})
ADD_LIBRARY(gwes_cpp ${srcs})

# executable gwes
message(STATUS "linking gwes with srcs=\"${srcs}\"")
add_executable(gwes ${srcs})
target_link_libraries(gwes gwes_cpp gworkflowdl_cpp xerces-c xml2 xslt fhglog)

install(FILES ${hdrs} DESTINATION include/gwes)
install(TARGETS gwes_cpp ARCHIVE DESTINATION lib)

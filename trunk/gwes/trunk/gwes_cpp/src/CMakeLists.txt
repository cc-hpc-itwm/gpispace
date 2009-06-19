
file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

# library gwes_cpp
ADD_LIBRARY(gwes_cpp ${hdrs} ${srcs})

# executable gwes
message(STATUS "linking gwes with srcs=\"${srcs}\"")
add_executable(gwes ${srcs})
target_link_libraries(gwes gwes_cpp gworkflowdl_cpp xerces-c)

install(FILES ${hdrs} DESTINATION include/gwes)
install(TARGETS gwes_cpp ARCHIVE DESTINATION lib)

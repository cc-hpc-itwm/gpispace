
file(GLOB gwes-hdrs "include/gwes/*.h" "include/gwes/*.hpp")
file(GLOB gwes-srcs "src/*.cpp")

# library gwes_cpp
ADD_LIBRARY(gwes_cpp ${gwes-srcs})

# executable gwes
message(STATUS "linking gwes with srcs=\"${gwes-srcs}\"")
add_executable(gwes ${gwes-srcs})
target_link_libraries(gwes gwes_cpp gworkflowdl_cpp xslt )
target_link_libraries(gwes_cpp xerces-c xml2 )

install(FILES ${gwes-hdrs} DESTINATION include/gwes)
install(TARGETS gwes gwes_cpp
  RUNTIME DESTINATION bin 
  ARCHIVE DESTINATION lib
)


file(GLOB gwes-hdrs "include/gwes/*.h" "include/gwes/*.hpp")
file(GLOB gwes-srcs "src/*.cpp")

# library gwes_cpp
ADD_LIBRARY(gwes_cpp ${gwes-srcs})
target_link_libraries(gwes_cpp gworkflowdl_cpp)
target_link_libraries(gwes_cpp ${UUID_LIBRARY})
target_link_libraries(gwes_cpp ${FHGLOG_LIBRARY})
target_link_libraries(gwes_cpp ${Boost_LIBRARIES})
target_link_libraries(gwes_cpp ${ZLIB_LIBRARIES})
target_link_libraries(gwes_cpp pthread)

# executable gwes
add_executable(gwes main.cpp)
target_link_libraries(gwes gwes_cpp)

install(FILES ${gwes-hdrs} DESTINATION include/gwes)
install(TARGETS gwes gwes_cpp
  RUNTIME DESTINATION bin 
  ARCHIVE DESTINATION lib
)

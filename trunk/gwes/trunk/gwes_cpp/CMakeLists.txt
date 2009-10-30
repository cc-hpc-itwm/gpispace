
file(GLOB gwes-hdrs "include/gwes/*.h" "include/gwes/*.hpp")
file(GLOB gwes-srcs "src/*.cpp")

# library gwes_cpp
ADD_LIBRARY(gwes_cpp ${gwes-srcs})

# executable gwes
add_executable(gwes main.cpp)
target_link_libraries(gwes gwes_cpp)
target_link_libraries(gwes gworkflowdl_cpp)
target_link_libraries(gwes pthread)
target_link_libraries(gwes ${XERCESC_LIBRARIES})
target_link_libraries(gwes ${LIBXML2_LIBRARIES})
target_link_libraries(gwes ${LIBXSLT_LIBRARIES})

install(FILES ${gwes-hdrs} DESTINATION include/gwes)
install(TARGETS gwes gwes_cpp
  RUNTIME DESTINATION bin 
  ARCHIVE DESTINATION lib
)

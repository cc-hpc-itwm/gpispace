include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})

add_executable(sdpac main.cpp)
target_link_libraries(sdpac sdpa)

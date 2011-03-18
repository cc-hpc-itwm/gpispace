
if (BUILD_TESTING)
  add_executable (test_we_type_bytearray test_bytearray.cpp)
  target_link_libraries(test_we_type_bytearray ${Boost_LIBRARIES})
  add_test ("test_we_type_bytearray" test_we_type_bytearray)
endif (BUILD_TESTING)

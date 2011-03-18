
if (BUILD_TESTING)
  add_executable (test_xml_stream test_xml_stream.cpp)
  target_link_libraries(test_xml_stream ${Boost_LIBRARIES})
  add_test ("test_xml_stream" test_xml_stream)
endif (BUILD_TESTING)

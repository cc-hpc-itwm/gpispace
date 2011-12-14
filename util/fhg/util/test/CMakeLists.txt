
if (BUILD_TESTING)

  add_executable (fhg_util_test_xml_stream test_xml_stream.cpp)
  target_link_libraries(fhg_util_test_xml_stream ${Boost_LIBRARIES})
  add_test ("test_xml_stream" fhg_util_test_xml_stream)

  add_executable (fhg_util_test_either test_either.cpp)
  add_test ("test_either" fhg_util_test_either)

endif (BUILD_TESTING)

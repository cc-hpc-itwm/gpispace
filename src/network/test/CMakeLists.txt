# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (basic.cpp PROJECT network BOOST_UNIT_TEST LINK_LIBRARIES net)

add_executable (multiple_binaries_binary multiple_binaries_binary.cpp)
target_link_libraries (multiple_binaries_binary net)

fhg_add_test (multiple_binaries.cpp
  PROJECT network
  BOOST_UNIT_TEST
  LINK_LIBRARIES net process fhg-util
  INCLUDE_DIRECTORIES PRIVATE ${CMAKE_SOURCE_DIR}
  DEPENDS multiple_binaries_binary

)

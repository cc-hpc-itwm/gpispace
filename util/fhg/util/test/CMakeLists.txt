# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (alphanum.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (num.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (remove_prefix.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (split.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (xml_stream.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (temporary_file.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (read_file.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (read_lines.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc fhg-util test-utilities Util::Generic
)

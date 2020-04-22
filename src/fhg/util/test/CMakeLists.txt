# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (NAME util_alphanum
  SOURCES alphanum.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_num
  SOURCES num.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_remove_prefix
  SOURCES remove_prefix.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_xml_stream
  SOURCES xml_stream.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_event
  SOURCES event.cpp
  USE_BOOST
  LIBRARIES Boost::thread Boost::system fhg-util
)

fhg_add_test (NAME util_queue
  SOURCES queue.cpp
  USE_BOOST
  LIBRARIES Boost::thread Boost::system fhg-util
)

fhg_add_test (NAME util_assert
  SOURCES assert.cpp
  USE_BOOST
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=1"
  LIBRARIES fhg-util
  )

fhg_add_test (NAME util_bounded_queue
  SOURCES bounded_queue.cpp
  USE_BOOST
  LIBRARIES Boost::thread Boost::system fhg-util
)
